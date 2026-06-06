#include "codegen_llvm.h"
#include <iostream>

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
    return llvm::Type::getVoidTy(context);
}

llvm::Function* CodegenLLVM::createFunction(const std::string& name, llvm::Type* returnType,
                                            const std::vector<std::pair<std::string, std::string>>& params) {
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : params) {
        paramTypes.push_back(getLLVMType(param.second));
    }
    
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module.get());
    
    return func;
}

bool CodegenLLVM::generate(ProgramAST* program, const std::string& outputFile) {
    // Генерируем код для всех функций
    for (const auto& func : program->getFunctions()) {
        llvm::Function* llvmFunc = generateFunction(func.get());
        if (llvmFunc) {
            functions[func->getName()] = llvmFunc;
        }
    }
    
    // Проверяем на ошибки
    llvm::verifyModule(*module, &llvm::errs());
    
    // Выводим в файл
    std::error_code ec;
    llvm::raw_fd_ostream dest(outputFile, ec, llvm::sys::fs::OF_None);
    if (ec) {
        std::cerr << "Error opening output file: " << ec.message() << "\n";
        return false;
    }
    
    module->print(dest, nullptr);
    dest.close();
    
    return true;
}

llvm::Function* CodegenLLVM::generateFunction(FunctionAST* func) {
    llvm::Type* returnType = getLLVMType(func->getReturnType());
    llvm::Function* llvmFunc = createFunction(func->getName(), returnType, func->getParams());
    
    // Создаем базовый блок для тела функции
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "entry", llvmFunc);
    builder->SetInsertPoint(entryBlock);
    
    // Сохраняем текущую функцию
    currentFunction = llvmFunc;
    
    // Регистрируем параметры функции
    size_t paramIndex = 0;
    for (auto& arg : llvmFunc->args()) {
        const std::string& paramName = func->getParams()[paramIndex].first;
        arg.setName(paramName);
        variables[paramName] = &arg;
        paramIndex++;
    }
    
    // Генерируем тело функции
    llvm::Value* bodyResult = generateStatement(&func->getBody());
    
    // Если тело не завершено return-ом, добавляем ret void
    if (func->getReturnType() == "void" && !builder->GetInsertBlock()->getTerminator()) {
        builder->CreateRetVoid();
    }
    
    return llvmFunc;
}

llvm::Value* CodegenLLVM::generateStatement(ASTNode* stmt) {
    if (auto* varDecl = dynamic_cast<VarDeclAST*>(stmt)) {
        // Обработка объявления переменной
        llvm::Type* type = getLLVMType(varDecl->getType());
        llvm::Value* alloc = builder->CreateAlloca(type, nullptr, varDecl->getName());
        
        variables[varDecl->getName()] = alloc;
        
        if (varDecl->getInit()) {
            llvm::Value* value = generateExpr(varDecl->getInit());
            if (value) {
                builder->CreateStore(value, alloc);
            }
        }
        
        return alloc;
    }
    else if (auto* assign = dynamic_cast<AssignExprAST*>(stmt)) {
        // Обработка присваивания
        if (variables.find(assign->getName()) != variables.end()) {
            llvm::Value* value = generateExpr(&assign->getValue());
            if (value) {
                builder->CreateStore(value, variables[assign->getName()]);
                return value;
            }
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
    if (auto* numExpr = dynamic_cast<NumberExprAST*>(expr)) {
        // Генерация числовых констант
        if (llvm::isa<llvm::IntegerType>(currentFunction->getFunctionType()->getReturnType())) {
            return builder->getInt32(static_cast<int>(numExpr->getValue()));
        } else {
            return llvm::ConstantFP::get(context, llvm::APFloat(numExpr->getValue()));
        }
    }
    else if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr)) {
        // Генерация переменных
        if (variables.find(varExpr->getName()) != variables.end()) {
            return builder->CreateLoad(variables[varExpr->getName()]->getType()->getPointerElementType(),
                                      variables[varExpr->getName()], varExpr->getName());
        }
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
    llvm::Value* lhs = generateExpr(&expr->getLHS());
    llvm::Value* rhs = generateExpr(&expr->getRHS());
    
    if (!lhs || !rhs) return nullptr;
    
    switch (expr->getOp()) {
        case '+':
            if (llvm::isa<llvm::IntegerType>(lhs->getType())) {
                return builder->CreateAdd(lhs, rhs, "addtmp");
            } else {
                return builder->CreateFAdd(lhs, rhs, "addtmp");
            }
        case '-':
            if (llvm::isa<llvm::IntegerType>(lhs->getType())) {
                return builder->CreateSub(lhs, rhs, "subtmp");
            } else {
                return builder->CreateFSub(lhs, rhs, "subtmp");
            }
        case '*':
            if (llvm::isa<llvm::IntegerType>(lhs->getType())) {
                return builder->CreateMul(lhs, rhs, "multmp");
            } else {
                return builder->CreateFMul(lhs, rhs, "multmp");
            }
        case '/':
            if (llvm::isa<llvm::IntegerType>(lhs->getType())) {
                return builder->CreateSDiv(lhs, rhs, "divtmp");
            } else {
                return builder->CreateFDiv(lhs, rhs, "divtmp");
            }
        case '<':
            return builder->CreateICmpSLT(lhs, rhs, "lttmp");
        case '>':
            return builder->CreateICmpSGT(lhs, rhs, "gttmp");
        case '=': // ==
            return builder->CreateICmpEQ(lhs, rhs, "eqtmp");
        default:
            std::cerr << "Unknown binary operator: " << expr->getOp() << "\n";
            return nullptr;
    }
}

llvm::Value* CodegenLLVM::generateCall(CallExprAST* call) {
    llvm::Function* callee = functions[call->getCallee()];
    if (!callee) {
        std::cerr << "Error: Function '" << call->getCallee() << "' not defined\n";
        return nullptr;
    }
    
    std::vector<llvm::Value*> args;
    for (const auto& arg : call->getArgs()) {
        llvm::Value* argValue = generateExpr(arg.get());
        if (!argValue) return nullptr;
        args.push_back(argValue);
    }
    
    return builder->CreateCall(callee, args, "calltmp");
}

llvm::Value* CodegenLLVM::generateIf(IfStmtAST* ifStmt) {
    llvm::Value* cond = generateExpr(&ifStmt->getCond());
    if (!cond) return nullptr;
    
    // Преобразуем условие к bool, если нужно
    if (!cond->getType()->isIntegerTy(1)) {
        cond = builder->CreateICmpNE(cond, builder->getInt32(0), "ifcond");
    }
    
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(context, "then", func);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(context, "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context, "ifend");
    
    builder->CreateCondBr(cond, thenBB, elseBB);
    
    // Блок then
    builder->SetInsertPoint(thenBB);
    llvm::Value* thenResult = generateStatement(ifStmt->getThenBranch());
    if (thenResult && !builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBB);
    }
    
    // Сохраняем текущую точку вставки
    llvm::BasicBlock* thenEndBB = builder->GetInsertBlock();
    
    // Блок else
    func->getBasicBlockList().push_back(elseBB);
    builder->SetInsertPoint(elseBB);
    
    llvm::Value* elseResult = nullptr;
    if (ifStmt->getElseBranch()) {
        elseResult = generateStatement(ifStmt->getElseBranch());
        if (elseResult && !builder->GetInsertBlock()->getTerminator()) {
            builder->CreateBr(mergeBB);
        }
    } else {
        builder->CreateBr(mergeBB);
    }
    
    // Блок объединения
    func->getBasicBlockList().push_back(mergeBB);
    builder->SetInsertPoint(mergeBB);
    
    return nullptr;
}

llvm::Value* CodegenLLVM::generateWhile(WhileStmtAST* whileStmt) {
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    
    llvm::BasicBlock* loopCondBB = llvm::BasicBlock::Create(context, "loopcond", func);
    llvm::BasicBlock* loopBodyBB = llvm::BasicBlock::Create(context, "loopbody");
    llvm::BasicBlock* loopEndBB = llvm::BasicBlock::Create(context, "loopend");
    
    // Переход к условию
    builder->CreateBr(loopCondBB);
    
    // Блок условия
    builder->SetInsertPoint(loopCondBB);
    llvm::Value* cond = generateExpr(&whileStmt->getCond());
    if (!cond) return nullptr;
    
    if (!cond->getType()->isIntegerTy(1)) {
        cond = builder->CreateICmpNE(cond, builder->getInt32(0), "whilecond");
    }
    
    builder->CreateCondBr(cond, loopBodyBB, loopEndBB);
    
    // Блок тела цикла
    func->getBasicBlockList().push_back(loopBodyBB);
    builder->SetInsertPoint(loopBodyBB);
    llvm::Value* bodyResult = generateStatement(&whileStmt->getBody());
    if (bodyResult && !builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(loopCondBB);
    }
    
    // Блок после цикла
    func->getBasicBlockList().push_back(loopEndBB);
    builder->SetInsertPoint(loopEndBB);
    
    return nullptr;
}

llvm::Value* CodegenLLVM::generateFor(ForStmtAST* forStmt) {
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    
    // Инициализация
    if (forStmt->getInit()) {
        generateStatement(forStmt->getInit());
    }
    
    llvm::BasicBlock* loopCondBB = llvm::BasicBlock::Create(context, "forcond", func);
    llvm::BasicBlock* loopBodyBB = llvm::BasicBlock::Create(context, "forbody");
    llvm::BasicBlock* loopIncBB = llvm::BasicBlock::Create(context, "forinc");
    llvm::BasicBlock* loopEndBB = llvm::BasicBlock::Create(context, "forend");
    
    // Переход к условию
    builder->CreateBr(loopCondBB);
    
    // Блок условия
    builder->SetInsertPoint(loopCondBB);
    llvm::Value* cond = generateExpr(&forStmt->getCond());
    if (!cond) return nullptr;
    
    if (!cond->getType()->isIntegerTy(1)) {
        cond = builder->CreateICmpNE(cond, builder->getInt32(0), "forcond");
    }
    
    builder->CreateCondBr(cond, loopBodyBB, loopEndBB);
    
    // Блок тела цикла
    func->getBasicBlockList().push_back(loopBodyBB);
    builder->SetInsertPoint(loopBodyBB);
    llvm::Value* bodyResult = generateStatement(&forStmt->getBody());
    if (bodyResult && !builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(loopIncBB);
    }
    
    // Блок инкремента
    func->getBasicBlockList().push_back(loopIncBB);
    builder->SetInsertPoint(loopIncBB);
    if (forStmt->getInc()) {
        generateExpr(&forStmt->getInc());
    }
    builder->CreateBr(loopCondBB);
    
    // Блок после цикла
    func->getBasicBlockList().push_back(loopEndBB);
    builder->SetInsertPoint(loopEndBB);
    
    return nullptr;
}

llvm::Value* CodegenLLVM::generateReturn(ReturnStmtAST* returnStmt) {
    if (returnStmt->getValue()) {
        llvm::Value* value = generateExpr(returnStmt->getValue());
        if (value) {
            return builder->CreateRet(value);
        }
    }
    return builder->CreateRetVoid();
}

llvm::Value* CodegenLLVM::generateBlock(BlockStmtAST* block) {
    llvm::Value* result = nullptr;
    for (const auto& stmt : block->getStatements()) {
        result = generateStatement(stmt.get());
        if (builder->GetInsertBlock()->getTerminator()) {
            break;
        }
    }
    return result;
}
