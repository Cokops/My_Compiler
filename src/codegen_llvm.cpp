#include "codegen_llvm.h"
#include <iostream>
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

CodegenLLVM::CodegenLLVM() {
    module = std::make_unique<llvm::Module>("my_compiler_module", context);
    builder = std::make_unique<llvm::IRBuilder<>>(context);
}

CodegenLLVM::~CodegenLLVM() {}

llvm::Type* CodegenLLVM::getLLVMType(const std::string& type) {
    if (type == "int") {
        return llvm::Type::getInt32Ty(context);
    } else if (type == "float") {
        return llvm::Type::getFloatTy(context);
    } else if (type == "bool") {
        return llvm::Type::getInt1Ty(context);
    } else if (type == "char") {
        return llvm::Type::getInt8Ty(context);
    } else if (type == "void") {
        return llvm::Type::getVoidTy(context);
    }
    return llvm::Type::getInt32Ty(context);
}

llvm::FunctionType* CodegenLLVM::getFunctionType(FunctionAST* func) {
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : func->getParams()) {
        paramTypes.push_back(getLLVMType(param.second));
    }
    llvm::Type* returnType = getLLVMType(func->getReturnType());
    return llvm::FunctionType::get(returnType, paramTypes, false);
}

bool CodegenLLVM::generate(ProgramAST* program, const std::string& outputFile) {
    for (const auto& func : program->getFunctions()) {
        llvm::FunctionType* funcType = getFunctionType(func);
        llvm::Function* llvmFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, func->getName(), module.get());
        functions[func->getName()] = llvmFunc;
        std::cout << "Created function: " << func->getName() << std::endl;
    }
    
    for (const auto& func : program->getFunctions()) {
        generateFunction(func);
    }
    
    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "Module verification failed\n";
        return false;
    }
    
    std::error_code ec;
    llvm::raw_fd_ostream dest(outputFile, ec);
    if (ec) {
        std::cerr << "Error opening output file: " << ec.message() << "\n";
        return false;
    }
    
    module->print(dest, nullptr);
    dest.close();
    
    return true;
}

llvm::Function* CodegenLLVM::generateFunction(FunctionAST* func) {
    std::cout << "Generating body for: " << func->getName() << std::endl;
    
    auto it = functions.find(func->getName());
    if (it == functions.end()) {
        std::cerr << "Error: Function '" << func->getName() << "' not found\n";
        return nullptr;
    }
    
    llvm::Function* llvmFunc = it->second;
    
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "entry", llvmFunc);
    builder->SetInsertPoint(entryBlock);
    
    currentFunction = llvmFunc;
    variables.clear();
    
    size_t paramIndex = 0;
    for (auto& arg : llvmFunc->args()) {
        const std::string& paramName = func->getParams()[paramIndex].first;
        const std::string& paramType = func->getParams()[paramIndex].second;
        arg.setName(paramName);
        
        llvm::AllocaInst* alloca = builder->CreateAlloca(getLLVMType(paramType), nullptr, paramName);
        builder->CreateStore(&arg, alloca);
        variables[paramName] = alloca;
        
        std::cout << "  Parameter: " << paramName << " (" << paramType << ")" << std::endl;
        paramIndex++;
    }
    
    generateStatement(func->getBody());
    
    if (!builder->GetInsertBlock()->getTerminator()) {
        if (func->getReturnType() == "void") {
            builder->CreateRetVoid();
        } else if (func->getReturnType() == "bool") {
            builder->CreateRet(llvm::ConstantInt::get(context, llvm::APInt(1, 0)));
        } else if (func->getReturnType() == "float") {
            builder->CreateRet(llvm::ConstantFP::get(context, llvm::APFloat(0.0f)));
        } else if (func->getReturnType() == "char") {
            builder->CreateRet(llvm::ConstantInt::get(context, llvm::APInt(8, 0)));
        } else {
            builder->CreateRet(llvm::ConstantInt::get(context, llvm::APInt(32, 0)));
        }
    }
    
    return llvmFunc;
}

llvm::Value* CodegenLLVM::generateStatement(ASTNode* stmt) {
    if (!stmt) return nullptr;
    
    if (auto* varDecl = dynamic_cast<VarDeclAST*>(stmt)) {
        llvm::Type* type = getLLVMType(varDecl->getType());
        llvm::AllocaInst* alloc = builder->CreateAlloca(type, nullptr, varDecl->getName());
        
        variables[varDecl->getName()] = alloc;
        std::cout << "  Variable declared: " << varDecl->getName() << " (" << varDecl->getType() << ")" << std::endl;
        
        if (varDecl->getInit()) {
            llvm::Value* value = generateExpr(varDecl->getInit());
            if (value) {
                // Преобразуем тип если нужно
                if (type->isIntegerTy(32) && value->getType()->isIntegerTy(1)) {
                    value = builder->CreateZExt(value, llvm::Type::getInt32Ty(context), "booltoint");
                } else if (type->isIntegerTy(1) && value->getType()->isIntegerTy(32)) {
                    value = builder->CreateICmpNE(value, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "inttobool");
                } else if (type->isFloatTy() && value->getType()->isIntegerTy(32)) {
                    value = builder->CreateSIToFP(value, llvm::Type::getFloatTy(context), "inttofloat");
                } else if (type->isIntegerTy(32) && value->getType()->isFloatTy()) {
                    value = builder->CreateFPToSI(value, llvm::Type::getInt32Ty(context), "floattoint");
                } else if (type->isIntegerTy(32) && value->getType()->isIntegerTy(8)) {
                    value = builder->CreateZExt(value, llvm::Type::getInt32Ty(context), "chartoint");
                } else if (type->isIntegerTy(8) && value->getType()->isIntegerTy(32)) {
                    value = builder->CreateTrunc(value, llvm::Type::getInt8Ty(context), "inttochar");
                }
                builder->CreateStore(value, alloc);
            }
        }
        
        return alloc;
    }
    else if (auto* assign = dynamic_cast<AssignExprAST*>(stmt)) {
        auto it = variables.find(assign->getName());
        if (it != variables.end()) {
            llvm::Value* value = generateExpr(assign->getValue());
            if (value) {
                llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(it->second);
                if (alloca) {
                    llvm::Type* varType = alloca->getAllocatedType();
                    if (varType->isIntegerTy(32) && value->getType()->isIntegerTy(1)) {
                        value = builder->CreateZExt(value, llvm::Type::getInt32Ty(context), "booltoint");
                    } else if (varType->isIntegerTy(1) && value->getType()->isIntegerTy(32)) {
                        value = builder->CreateICmpNE(value, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "inttobool");
                    } else if (varType->isIntegerTy(32) && value->getType()->isIntegerTy(8)) {
                        value = builder->CreateZExt(value, llvm::Type::getInt32Ty(context), "chartoint");
                    } else if (varType->isIntegerTy(8) && value->getType()->isIntegerTy(32)) {
                        value = builder->CreateTrunc(value, llvm::Type::getInt8Ty(context), "inttochar");
                    }
                }
                builder->CreateStore(value, it->second);
                return value;
            }
        } else {
            std::cerr << "Error: Variable '" << assign->getName() << "' not declared" << std::endl;
        }
        return nullptr;
    }
    else if (auto* ifStmt = dynamic_cast<IfStmtAST*>(stmt)) {
        return generateIf(ifStmt);
    }
    else if (auto* whileStmt = dynamic_cast<WhileStmtAST*>(stmt)) {
        return generateWhile(whileStmt);
    }
    else if (auto* forStmt = dynamic_cast<ForStmtAST*>(stmt)) {
        return generateFor(forStmt);
    }
    else if (auto* returnStmt = dynamic_cast<ReturnStmtAST*>(stmt)) {
        return generateReturn(returnStmt);
    }
    else if (auto* block = dynamic_cast<BlockStmtAST*>(stmt)) {
        return generateBlock(block);
    }
    
    return nullptr;
}

llvm::Value* CodegenLLVM::generateExpr(ExprAST* expr) {
    if (!expr) return nullptr;
    
    if (auto* numExpr = dynamic_cast<NumberExprAST*>(expr)) {
        if (numExpr->getIsFloat()) {
            return llvm::ConstantFP::get(context, llvm::APFloat((float)numExpr->getValue()));
        } else {
            return llvm::ConstantInt::get(context, llvm::APInt(32, (int)numExpr->getValue()));
        }
    }
    else if (auto* boolExpr = dynamic_cast<BoolExprAST*>(expr)) {
        return llvm::ConstantInt::get(context, llvm::APInt(1, boolExpr->getValue() ? 1 : 0));
    }
    else if (auto* charExpr = dynamic_cast<CharExprAST*>(expr)) {
        return llvm::ConstantInt::get(context, llvm::APInt(8, charExpr->getValue()));
    }
    else if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr)) {
        auto it = variables.find(varExpr->getName());
        if (it != variables.end()) {
            llvm::AllocaInst* alloca = (llvm::AllocaInst*)it->second;
            return builder->CreateLoad(alloca->getAllocatedType(), alloca, varExpr->getName());
        }
        std::cerr << "Error: Variable '" << varExpr->getName() << "' not found" << std::endl;
        return nullptr;
    }
    else if (auto* binExpr = dynamic_cast<BinaryExprAST*>(expr)) {
        return generateBinaryExpr(binExpr);
    }
    else if (auto* callExpr = dynamic_cast<CallExprAST*>(expr)) {
        return generateCall(callExpr);
    }
    
    return nullptr;
}

llvm::Value* CodegenLLVM::generateBinaryExpr(BinaryExprAST* expr) {
    llvm::Value* lhs = generateExpr(expr->getLHS());
    llvm::Value* rhs = generateExpr(expr->getRHS());
    
    if (!lhs || !rhs) return nullptr;
    
    int op = expr->getOp();
    
    // Логические операции и сравнения
    switch (op) {
        case 278:   // EQ
            return builder->CreateICmpEQ(lhs, rhs, "eqtmp");
        case 279:   // NEQ
            return builder->CreateICmpNE(lhs, rhs, "netmp");
        case 280:   // LT
            return builder->CreateICmpSLT(lhs, rhs, "lttmp");
        case 281:   // GT
            return builder->CreateICmpSGT(lhs, rhs, "gttmp");
        case 282:   // LE
            return builder->CreateICmpSLE(lhs, rhs, "letmp");
        case 283:   // GE
            return builder->CreateICmpSGE(lhs, rhs, "getmp");
        case 284:   // AND (&&)
            if (!lhs->getType()->isIntegerTy(1)) {
                lhs = builder->CreateICmpNE(lhs, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "lhsbool");
            }
            if (!rhs->getType()->isIntegerTy(1)) {
                rhs = builder->CreateICmpNE(rhs, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "rhsbool");
            }
            return builder->CreateAnd(lhs, rhs, "andtmp");
        case 285:   // OR (||)
            if (!lhs->getType()->isIntegerTy(1)) {
                lhs = builder->CreateICmpNE(lhs, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "lhsbool");
            }
            if (!rhs->getType()->isIntegerTy(1)) {
                rhs = builder->CreateICmpNE(rhs, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "rhsbool");
            }
            return builder->CreateOr(lhs, rhs, "ortmp");
        default:
            break;
    }
    
    // Арифметические операции с float
    if (lhs->getType()->isFloatTy() || rhs->getType()->isFloatTy()) {
        switch (op) {
            case 273: return builder->CreateFAdd(lhs, rhs, "addtmpf");
            case 274: return builder->CreateFSub(lhs, rhs, "subtmpf");
            case 275: return builder->CreateFMul(lhs, rhs, "multmpf");
            case 276: return builder->CreateFDiv(lhs, rhs, "divtmpf");
            default: break;
        }
    } 
    // Арифметические операции с int/char
    else {
        switch (op) {
            case 273: return builder->CreateAdd(lhs, rhs, "addtmp");
            case 274: return builder->CreateSub(lhs, rhs, "subtmp");
            case 275: return builder->CreateMul(lhs, rhs, "multmp");
            case 276: return builder->CreateSDiv(lhs, rhs, "divtmp");
            default: break;
        }
    }
    
    std::cerr << "Unknown binary operator: " << op << "\n";
    return nullptr;
}

llvm::Value* CodegenLLVM::generateCall(CallExprAST* call) {
    auto it = functions.find(call->getCallee());
    if (it == functions.end()) {
        std::cerr << "Error: Function '" << call->getCallee() << "' not defined\n";
        return nullptr;
    }
    
    llvm::Function* callee = it->second;
    std::vector<llvm::Value*> args;
    
    for (const auto& arg : call->getArgs()) {
        llvm::Value* argValue = generateExpr(arg);
        if (!argValue) return nullptr;
        
        if (argValue->getType() != callee->getFunctionType()->getParamType(args.size())) {
            if (argValue->getType()->isIntegerTy(1) && callee->getFunctionType()->getParamType(args.size())->isIntegerTy(32)) {
                argValue = builder->CreateZExt(argValue, llvm::Type::getInt32Ty(context), "zext");
            } else if (argValue->getType()->isIntegerTy(8) && callee->getFunctionType()->getParamType(args.size())->isIntegerTy(32)) {
                argValue = builder->CreateZExt(argValue, llvm::Type::getInt32Ty(context), "chartoint");
            }
        }
        args.push_back(argValue);
    }
    
    return builder->CreateCall(callee, args, "calltmp");
}

llvm::Value* CodegenLLVM::generateIf(IfStmtAST* ifStmt) {
    llvm::Value* cond = generateExpr(ifStmt->getCond());
    if (!cond) return nullptr;
    
    if (cond->getType()->isIntegerTy(32)) {
        cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "ifcond");
    }
    
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(context, "then", func);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(context, "else", func);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context, "ifend", func);
    
    builder->CreateCondBr(cond, thenBB, elseBB);
    
    builder->SetInsertPoint(thenBB);
    generateStatement(ifStmt->getThenBranch());
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBB);
    }
    
    builder->SetInsertPoint(elseBB);
    if (ifStmt->getElseBranch()) {
        generateStatement(ifStmt->getElseBranch());
    }
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBB);
    }
    
    builder->SetInsertPoint(mergeBB);
    
    return nullptr;
}

llvm::Value* CodegenLLVM::generateWhile(WhileStmtAST* whileStmt) {
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    
    llvm::BasicBlock* loopCondBB = llvm::BasicBlock::Create(context, "loopcond", func);
    llvm::BasicBlock* loopBodyBB = llvm::BasicBlock::Create(context, "loopbody", func);
    llvm::BasicBlock* loopEndBB = llvm::BasicBlock::Create(context, "loopend", func);
    
    builder->CreateBr(loopCondBB);
    
    builder->SetInsertPoint(loopCondBB);
    llvm::Value* cond = generateExpr(whileStmt->getCond());
    if (!cond) return nullptr;
    
    if (cond->getType()->isIntegerTy(32)) {
        cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "whilecond");
    }
    
    builder->CreateCondBr(cond, loopBodyBB, loopEndBB);
    
    builder->SetInsertPoint(loopBodyBB);
    generateStatement(whileStmt->getBody());
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(loopCondBB);
    }
    
    builder->SetInsertPoint(loopEndBB);
    
    return nullptr;
}

llvm::Value* CodegenLLVM::generateFor(ForStmtAST* forStmt) {
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    
    if (forStmt->getInit()) {
        generateStatement(forStmt->getInit());
    }
    
    llvm::BasicBlock* loopCondBB = llvm::BasicBlock::Create(context, "forcond", func);
    llvm::BasicBlock* loopBodyBB = llvm::BasicBlock::Create(context, "forbody", func);
    llvm::BasicBlock* loopIncBB = llvm::BasicBlock::Create(context, "forinc", func);
    llvm::BasicBlock* loopEndBB = llvm::BasicBlock::Create(context, "forend", func);
    
    builder->CreateBr(loopCondBB);
    
    builder->SetInsertPoint(loopCondBB);
    llvm::Value* cond = generateExpr(forStmt->getCond());
    if (!cond) return nullptr;
    
    if (cond->getType()->isIntegerTy(32)) {
        cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "forcond");
    }
    
    builder->CreateCondBr(cond, loopBodyBB, loopEndBB);
    
    builder->SetInsertPoint(loopBodyBB);
    generateStatement(forStmt->getBody());
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(loopIncBB);
    }
    
    builder->SetInsertPoint(loopIncBB);
    if (forStmt->getInc()) {
        generateExpr(forStmt->getInc());
    }
    builder->CreateBr(loopCondBB);
    
    builder->SetInsertPoint(loopEndBB);
    
    return nullptr;
}

llvm::Value* CodegenLLVM::generateReturn(ReturnStmtAST* returnStmt) {
    if (returnStmt->getValue()) {
        llvm::Value* value = generateExpr(returnStmt->getValue());
        if (value) {
            if (currentFunction->getReturnType()->isIntegerTy(1) && value->getType()->isIntegerTy(32)) {
                value = builder->CreateICmpNE(value, llvm::ConstantInt::get(context, llvm::APInt(32, 0)), "inttobool");
            } else if (currentFunction->getReturnType()->isIntegerTy(32) && value->getType()->isIntegerTy(1)) {
                value = builder->CreateZExt(value, llvm::Type::getInt32Ty(context), "booltoint");
            } else if (currentFunction->getReturnType()->isIntegerTy(32) && value->getType()->isIntegerTy(8)) {
                value = builder->CreateZExt(value, llvm::Type::getInt32Ty(context), "chartoint");
            } else if (currentFunction->getReturnType()->isIntegerTy(8) && value->getType()->isIntegerTy(32)) {
                value = builder->CreateTrunc(value, llvm::Type::getInt8Ty(context), "inttochar");
            }
            return builder->CreateRet(value);
        }
    }
    if (currentFunction->getReturnType()->isVoidTy()) {
        return builder->CreateRetVoid();
    } else if (currentFunction->getReturnType()->isIntegerTy(1)) {
        return builder->CreateRet(llvm::ConstantInt::get(context, llvm::APInt(1, 0)));
    } else if (currentFunction->getReturnType()->isIntegerTy(8)) {
        return builder->CreateRet(llvm::ConstantInt::get(context, llvm::APInt(8, 0)));
    } else {
        return builder->CreateRet(llvm::ConstantInt::get(context, llvm::APInt(32, 0)));
    }
}

llvm::Value* CodegenLLVM::generateBlock(BlockStmtAST* block) {
    llvm::Value* result = nullptr;
    for (const auto& stmt : block->getStatements()) {
        result = generateStatement(stmt);
        if (builder->GetInsertBlock()->getTerminator()) {
            break;
        }
    }
    return result;
}