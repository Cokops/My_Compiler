#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include <map>
#include <vector>
#include <string>
#include <memory>

// Класс для хранения информации о типах переменных
class TypeTable {
public:
    void addVariable(const std::string& name, const std::string& type) {
        variables[name] = type;
    }
    
    std::string getVariableType(const std::string& name) const {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second;
        }
        return "";
    }
    
    bool hasVariable(const std::string& name) const {
        return variables.find(name) != variables.end();
    }
    
private:
    std::map<std::string, std::string> variables;
};

// Класс для хранения информации о функциях
class FunctionTable {
public:
    struct FunctionInfo {
        std::string returnType;
        std::vector<std::pair<std::string, std::string>> params;
    };
    
    void addFunction(const std::string& name, const FunctionInfo& info) {
        functions[name] = info;
    }
    
    FunctionInfo getFunction(const std::string& name) const {
        auto it = functions.find(name);
        if (it != functions.end()) {
            return it->second;
        }
        return {};
    }
    
    bool hasFunction(const std::string& name) const {
        return functions.find(name) != functions.end();
    }
    
private:
    std::map<std::string, FunctionInfo> functions;
};

// Семантический анализатор
class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    
    // Проанализировать программу
    bool analyze(ProgramAST* program);
    
    // Проанализировать функцию
    bool analyzeFunction(FunctionAST* func);
    
    // Проанализировать оператор
    bool analyzeStatement(ASTNode* stmt);
    
    // Проанализировать выражение
    bool analyzeExpr(ExprAST* expr);
    
    // Получить тип выражения
    std::string getExprType(ExprAST* expr);
    
    // Проверить совместимость типов
    bool checkTypeCompatibility(const std::string& expected, const std::string& actual);
    
private:
    TypeTable typeTable;
    FunctionTable functionTable;
    
    // Области видимости
    std::vector<TypeTable> scopeStack;
    
    // Проверить объявление переменной
    bool checkVarDecl(VarDeclAST* decl);
    
    // Проверить присваивание
    bool checkAssignment(AssignExprAST* assign);
    
    // Проверить вызов функции
    bool checkCall(CallExprAST* call);
};

#endif // SEMANTIC_ANALYZER_H
