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
        llvm::Function* llvmFunc = generateFunction(const_cast<FunctionAST*>(func.get()));
        if (llvmFunc) {
            functions[func->getName()] = llvmFunc;
        }
    }
    
    // Проверяем на ошибки
    llvm::verifyModule(*module, &llvm::errs());
    
    // Выводим в файл (исправлено для LLVM 18)
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
    llvm::Value* bodyResult = generateStatement(const_cast<ASTNode*>(&func->getBody()));
    
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
            llvm::Value* value = generateExpr(const_cast<ExprAST*>(varDecl->getInit()));
            if (value) {
                builder->CreateStore(value, alloc);
            }
        }
        
        return alloc;
    }
    else if (auto* assign = dynamic_cast<AssignExprAST*>(stmt)) {
        // Обработка присваивания
        if (variables.find(assign->getName()) != variables.end()) {
            llvm::Value* value = generateExpr(const_cast<ExprAST*>(&assign->getValue()));
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
        return llvm::ConstantInt::get(context, llvm::APInt(32, static_cast<int>(numExpr->getValue())));
    }
    else if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr)) {
        // Генерация переменных
        if (variables.find(varExpr->getName()) != variables.end()) {
            llvm::Value* ptr = variables[varExpr->getName()];
            return builder->CreateLoad(llvm::Type::getInt32Ty(context), ptr, varExpr->getName());
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
    llvm::Value* lhs = generateExpr(const_cast<ExprAST*>(&expr->getLHS()));
    llvm::Value* rhs = generateExpr(const_cast<ExprAST*>(&expr->getRHS()));
    
    if (!lhs || !rhs) return nullptr;
    
    switch (expr->getOp()) {
        case '+':
            return builder->CreateAdd(lhs, rhs, "addtmp");
        case '-':
            return builder->CreateSub(lhs, rhs, "subtmp");
        case '*':
            return builder->CreateMul(lhs, rhs, "multmp");
        case '/':
            return builder->CreateSDiv(lhs, rhs, "divtmp");
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
        llvm::Value* argValue = generateExpr(const_cast<ExprAST*>(arg.get()));
        if (!argValue) return nullptr;
        args.push_back(argValue);
    }
    
    return builder->CreateCall(callee, args, "calltmp");
}

llvm::Value* CodegenLLVM::generateIf(IfStmtAST* ifStmt) {
    llvm::Value* cond = generateExpr(const_cast<ExprAST*>(&ifStmt->getCond()));
    if (!cond) return nullptr;
    
    // Преобразуем условие к bool, если нужно
    if (!cond->getType()->isIntegerTy(1)) {
        cond = builder->CreateICmpNE(cond, builder->getInt32(0), "ifcond");
    }
    
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(context, "then", func);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(context, "else", func);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(context, "ifend", func);
    
    builder->CreateCondBr(cond, thenBB, elseBB);
    
    // Блок then
    builder->SetInsertPoint(thenBB);
    llvm::Value* thenResult = generateStatement(const_cast<ASTNode*>(ifStmt->getThenBranch()));
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBB);
    }
    
    // Блок else
    builder->SetInsertPoint(elseBB);
    if (ifStmt->getElseBranch()) {
        generateStatement(const_cast<ASTNode*>(ifStmt->getElseBranch()));
    }
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBB);
    }
    
    // Блок объединения
    builder->SetInsertPoint(mergeBB);
    
    return nullptr;
}

llvm::Value* CodegenLLVM::generateWhile(WhileStmtAST* whileStmt) {
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    
    llvm::BasicBlock* loopCondBB = llvm::BasicBlock::Create(context, "loopcond", func);
    llvm::BasicBlock* loopBodyBB = llvm::BasicBlock::Create(context, "loopbody", func);
    llvm::BasicBlock* loopEndBB = llvm::BasicBlock::Create(context, "loopend", func);
    
    // Переход к условию
    builder->CreateBr(loopCondBB);
    
    // Блок условия
    builder->SetInsertPoint(loopCondBB);
    llvm::Value* cond = generateExpr(const_cast<ExprAST*>(&whileStmt->getCond()));
    if (!cond) return nullptr;
    
    if (!cond->getType()->isIntegerTy(1)) {
        cond = builder->CreateICmpNE(cond, builder->getInt32(0), "whilecond");
    }
    
    builder->CreateCondBr(cond, loopBodyBB, loopEndBB);
    
    // Блок тела цикла
    builder->SetInsertPoint(loopBodyBB);
    generateStatement(const_cast<ASTNode*>(&whileStmt->getBody()));
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(loopCondBB);
    }
    
    // Блок после цикла
    builder->SetInsertPoint(loopEndBB);
    
    return nullptr;
}

llvm::Value* CodegenLLVM::generateFor(ForStmtAST* forStmt) {
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    
    // Инициализация
    if (forStmt->getInit()) {
        generateStatement(const_cast<VarDeclAST*>(forStmt->getInit()));
    }
    
    llvm::BasicBlock* loopCondBB = llvm::BasicBlock::Create(context, "forcond", func);
    llvm::BasicBlock* loopBodyBB = llvm::BasicBlock::Create(context, "forbody", func);
    llvm::BasicBlock* loopIncBB = llvm::BasicBlock::Create(context, "forinc", func);
    llvm::BasicBlock* loopEndBB = llvm::BasicBlock::Create(context, "forend", func);
    
    // Переход к условию
    builder->CreateBr(loopCondBB);
    
    // Блок условия
    builder->SetInsertPoint(loopCondBB);
    llvm::Value* cond = generateExpr(const_cast<ExprAST*>(&forStmt->getCond()));
    if (!cond) return nullptr;
    
    if (!cond->getType()->isIntegerTy(1)) {
        cond = builder->CreateICmpNE(cond, builder->getInt32(0), "forcond");
    }
    
    builder->CreateCondBr(cond, loopBodyBB, loopEndBB);
    
    // Блок тела цикла
    builder->SetInsertPoint(loopBodyBB);
    generateStatement(const_cast<ASTNode*>(&forStmt->getBody()));
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(loopIncBB);
    }
    
    // Блок инкремента - убрана проверка if, так как getInc() возвращает ссылку
    builder->SetInsertPoint(loopIncBB);
    generateExpr(const_cast<ExprAST*>(&forStmt->getInc()));
    builder->CreateBr(loopCondBB);
    
    // Блок после цикла
    builder->SetInsertPoint(loopEndBB);
    
    return nullptr;
}

llvm::Value* CodegenLLVM::generateReturn(ReturnStmtAST* returnStmt) {
    if (returnStmt->getValue()) {
        llvm::Value* value = generateExpr(const_cast<ExprAST*>(returnStmt->getValue()));
        if (value) {
            return builder->CreateRet(value);
        }
    }
    return builder->CreateRetVoid();
}

llvm::Value* CodegenLLVM::generateBlock(BlockStmtAST* block) {
    llvm::Value* result = nullptr;
    for (const auto& stmt : block->getStatements()) {
        result = generateStatement(const_cast<ASTNode*>(stmt.get()));
        if (builder->GetInsertBlock()->getTerminator()) {
            break;
        }
    }
    return result;
}