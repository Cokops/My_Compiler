#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include <string>
#include <vector>
#include <map>

// Таблица типов для области видимости
class TypeTable {
public:
    void addVariable(const std::string& name, const std::string& type) {
        variables[name] = type;
    }
    
    bool hasVariable(const std::string& name) const {
        return variables.find(name) != variables.end();
    }
    
    std::string getVariableType(const std::string& name) const {
        auto it = variables.find(name);
        return it != variables.end() ? it->second : "";
    }
    
private:
    std::map<std::string, std::string> variables;
};

// Таблица функций
class FunctionTable {
public:
    struct FunctionInfo {
        std::string returnType;
        std::vector<std::pair<std::string, std::string>> params;
    };
    
    void addFunction(const std::string& name, const FunctionInfo& info) {
        functions[name] = info;
    }
    
    bool hasFunction(const std::string& name) const {
        return functions.find(name) != functions.end();
    }
    
    FunctionInfo getFunction(const std::string& name) const {
        return functions.at(name);
    }
    
private:
    std::map<std::string, FunctionInfo> functions;
};

// Класс семантического анализатора
class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    
    bool analyze(ProgramAST* program);
    
private:
    std::vector<TypeTable> scopeStack;
    FunctionTable functionTable;
    
    bool analyzeFunction(FunctionAST* func);
    bool analyzeStatement(ASTNode* stmt);
    bool analyzeExpr(ExprAST* expr);
    
    std::string getExprType(ExprAST* expr);
    bool checkTypeCompatibility(const std::string& expected, const std::string& actual);
    
    bool checkVarDecl(VarDeclAST* decl);
    bool checkAssignment(AssignExprAST* assign);
    bool checkCall(CallExprAST* call);
};

#endif // SEMANTIC_ANALYZER_H