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
    // Сначала создаем объявления ВСЕХ функций
    for (const auto& func : program->getFunctions()) {
        llvm::FunctionType* funcType = getFunctionType(func);
        llvm::Function* llvmFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, func->getName(), module.get());
        functions[func->getName()] = llvmFunc;
        std::cout << "Created function: " << func->getName() << std::endl;
    }
    
    // Затем генерируем тела функций
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
    
    // Создаем базовый блок
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "entry", llvmFunc);
    builder->SetInsertPoint(entryBlock);
    
    currentFunction = llvmFunc;
    
    // ОЧИЩАЕМ переменные для этой функции
    variables.clear();
    
    // Регистрируем параметры функции в таблицу переменных
    size_t paramIndex = 0;
    for (auto& arg : llvmFunc->args()) {
        const std::string& paramName = func->getParams()[paramIndex].first;
        const std::string& paramType = func->getParams()[paramIndex].second;
        arg.setName(paramName);
        
        // Создаем alloca для параметра
        llvm::AllocaInst* alloca = builder->CreateAlloca(getLLVMType(paramType), nullptr, paramName);
        builder->CreateStore(&arg, alloca);
        variables[paramName] = alloca;
        
        std::cout << "  Parameter: " << paramName << " stored in alloca" << std::endl;
        paramIndex++;
    }
    
    // Генерируем тело функции
    generateStatement(func->getBody());
    
    // Добавляем return если нужно
    if (!builder->GetInsertBlock()->getTerminator()) {
        if (func->getReturnType() == "void") {
            builder->CreateRetVoid();
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
        std::cout << "  Variable declared: " << varDecl->getName() << std::endl;
        
        if (varDecl->getInit()) {
            llvm::Value* value = generateExpr(varDecl->getInit());
            if (value) {
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
        return llvm::ConstantInt::get(context, llvm::APInt(32, static_cast<int>(numExpr->getValue())));
    }
    else if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr)) {
        auto it = variables.find(varExpr->getName());
        if (it != variables.end()) {
            // Загружаем значение из alloca
            llvm::AllocaInst* alloca = (llvm::AllocaInst*)it->second;
            return builder->CreateLoad(alloca->getAllocatedType(), alloca, varExpr->getName());
        }
        std::cerr << "Error: Variable '" << varExpr->getName() << "' not found in variables table" << std::endl;
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
    
    switch (op) {
        case 272: return builder->CreateAdd(lhs, rhs, "addtmp");
        case 273: return builder->CreateSub(lhs, rhs, "subtmp");
        case 274: return builder->CreateMul(lhs, rhs, "multmp");
        case 275: return builder->CreateSDiv(lhs, rhs, "divtmp");
        case 276: 
        case 277: return builder->CreateICmpEQ(lhs, rhs, "eqtmp");
        case 278: return builder->CreateICmpNE(lhs, rhs, "netmp");
        case 279: return builder->CreateICmpSLT(lhs, rhs, "lttmp");
        case 280: return builder->CreateICmpSGT(lhs, rhs, "gttmp");
        case 281: return builder->CreateICmpSLE(lhs, rhs, "letmp");
        case 282: return builder->CreateICmpSGE(lhs, rhs, "getmp");
        case 283: return builder->CreateAnd(lhs, rhs, "andtmp");
        case 284: return builder->CreateOr(lhs, rhs, "ortmp");
        default:
            std::cerr << "Unknown binary operator: " << op << "\n";
            return nullptr;
    }
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
            std::cout << "Returning value" << std::endl;
            return builder->CreateRet(value);
        }
    }
    std::cout << "Returning void" << std::endl;
    return builder->CreateRetVoid();
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