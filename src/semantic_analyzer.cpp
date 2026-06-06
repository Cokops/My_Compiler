#include "semantic_analyzer.h"
#include <iostream>

SemanticAnalyzer::SemanticAnalyzer() {
    // Добавим базовые функции
    FunctionTable::FunctionInfo printInfo;
    printInfo.returnType = "void";
    printInfo.params.push_back(std::make_pair("value", "any"));
    functionTable.addFunction("print", printInfo);
}

bool SemanticAnalyzer::analyze(ProgramAST* program) {
    // Сначала регистрируем все функции
    for (const auto& func : program->getFunctions()) {
        FunctionTable::FunctionInfo info;
        info.returnType = func->getReturnType();
        info.params = func->getParams();
        functionTable.addFunction(func->getName(), info);
    }
    
    // Затем анализируем тела функций
    for (const auto& func : program->getFunctions()) {
        if (!analyzeFunction(func.get())) {
            return false;
        }
    }
    
    return true;
}

bool SemanticAnalyzer::analyzeFunction(FunctionAST* func) {
    // Создаем новую область видимости
    scopeStack.push_back(TypeTable());
    
    // Регистрируем параметры функции
    for (const auto& param : func->getParams()) {
        scopeStack.back().addVariable(param.first, param.second);
    }
    
    // Анализируем тело функции
    bool result = analyzeStatement(&func->getBody());
    
    // Удаляем область видимости
    scopeStack.pop_back();
    
    return result;
}

bool SemanticAnalyzer::analyzeStatement(ASTNode* stmt) {
    if (auto* varDecl = dynamic_cast<VarDeclAST*>(stmt)) {
        return checkVarDecl(varDecl);
    }
    else if (auto* assign = dynamic_cast<AssignExprAST*>(stmt)) {
        return checkAssignment(assign);
    }
    else if (auto* ifStmt = dynamic_cast<IfStmtAST*>(stmt)) {
        if (!analyzeExpr(&ifStmt->getCond())) {
            std::cerr << "Error: Invalid condition in if statement\n";
            return false;
        }
        if (!analyzeStatement(ifStmt->getThenBranch())) {
            return false;
        }
        if (ifStmt->getElseBranch() && !analyzeStatement(ifStmt->getElseBranch())) {
            return false;
        }
        return true;
    }
    else if (auto* whileStmt = dynamic_cast<WhileStmtAST*>(stmt)) {
        if (!analyzeExpr(&whileStmt->getCond())) {
            std::cerr << "Error: Invalid condition in while statement\n";
            return false;
        }
        return analyzeStatement(&whileStmt->getBody());
    }
    else if (auto* forStmt = dynamic_cast<ForStmtAST*>(stmt)) {
        if (forStmt->getInit() && !analyzeStatement(forStmt->getInit())) {
            return false;
        }
        if (!analyzeExpr(&forStmt->getCond())) {
            std::cerr << "Error: Invalid condition in for statement\n";
            return false;
        }
        if (!analyzeExpr(&forStmt->getInc())) {
            std::cerr << "Error: Invalid increment in for statement\n";
            return false;
        }
        return analyzeStatement(&forStmt->getBody());
    }
    else if (auto* returnStmt = dynamic_cast<ReturnStmtAST*>(stmt)) {
        if (returnStmt->getValue() && !analyzeExpr(returnStmt->getValue())) {
            return false;
        }
        return true;
    }
    else if (auto* block = dynamic_cast<BlockStmtAST*>(stmt)) {
        for (const auto& s : block->getStatements()) {
            if (!analyzeStatement(s.get())) {
                return false;
            }
        }
        return true;
    }
    
    return true;
}

bool SemanticAnalyzer::analyzeExpr(ExprAST* expr) {
    // Для базовых выражений просто возвращаем true
    // Проверка типов будет в конкретных реализациях
    return true;
}

std::string SemanticAnalyzer::getExprType(ExprAST* expr) {
    if (dynamic_cast<NumberExprAST*>(expr)) {
        return "int";
    }
    else if (dynamic_cast<StringExprAST*>(expr)) {
        return "string";
    }
    else if (auto* var = dynamic_cast<VariableExprAST*>(expr)) {
        return typeTable.getVariableType(var->getName());
    }
    return "unknown";
}

bool SemanticAnalyzer::checkTypeCompatibility(const std::string& expected, const std::string& actual) {
    // Простая проверка совместимости типов
    return expected == actual || expected == "any" || actual == "any";
}

bool SemanticAnalyzer::checkVarDecl(VarDeclAST* decl) {
    std::string type = decl->getType();
    std::string name = decl->getName();
    
    // Проверяем, не объявлен ли уже такой идентификатор
    if (scopeStack.back().hasVariable(name)) {
        std::cerr << "Error: Variable '" << name << "' already declared\n";
        return false;
    }
    
    // Регистрируем переменную
    scopeStack.back().addVariable(name, type);
    
    // Если есть инициализация, проверяем тип
    if (decl->getInit()) {
        std::string initType = getExprType(decl->getInit());
        if (!initType.empty() && initType != "unknown" && !checkTypeCompatibility(type, initType)) {
            std::cerr << "Error: Type mismatch in initialization of '" << name << "'\n";
            return false;
        }
    }
    
    return true;
}

bool SemanticAnalyzer::checkAssignment(AssignExprAST* assign) {
    std::string name = assign->getName();
    
    // Проверяем, существует ли переменная
    if (!scopeStack.back().hasVariable(name)) {
        std::cerr << "Error: Undefined variable '" << name << "'\n";
        return false;
    }
    
    // Проверяем тип правой части
    std::string varType = scopeStack.back().getVariableType(name);
    std::string exprType = getExprType(&assign->getValue());
    
    if (!exprType.empty() && exprType != "unknown" && !checkTypeCompatibility(varType, exprType)) {
        std::cerr << "Error: Type mismatch in assignment to '" << name << "'\n";
        return false;
    }
    
    return true;
}

bool SemanticAnalyzer::checkCall(CallExprAST* call) {
    std::string name = call->getCallee();
    
    // Проверяем, существует ли функция
    if (!functionTable.hasFunction(name)) {
        std::cerr << "Error: Undefined function '" << name << "'\n";
        return false;
    }
    
    // Проверяем количество аргументов
    auto funcInfo = functionTable.getFunction(name);
    if (funcInfo.params.size() != call->getArgs().size()) {
        std::cerr << "Error: Function '" << name << "' expects " 
                  << funcInfo.params.size() << " arguments, but got " 
                  << call->getArgs().size() << "\n";
        return false;
    }
    
    // Проверяем типы аргументов
    for (size_t i = 0; i < call->getArgs().size(); ++i) {
        std::string argType = getExprType(call->getArgs()[i].get());
        if (!argType.empty() && argType != "unknown" && !checkTypeCompatibility(funcInfo.params[i].second, argType)) {
            std::cerr << "Error: Type mismatch in argument " << i << " of function '" << name << "'\n";
            return false;
        }
    }
    
    return true;
}
