#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>

// Базовый класс для всех узлов AST
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void codegen() = 0;  // чисто виртуальный
    virtual void print(int depth = 0) const = 0;
protected:
    static void indent(int depth) {
        for (int i = 0; i < depth; ++i) std::cout << "  ";
    }
};

// Выражения
class ExprAST : public ASTNode {
public:
    virtual ~ExprAST() = default;
    void print(int depth = 0) const override;
};

// Числовые константы
class NumberExprAST : public ExprAST {
    double Val;
public:
    NumberExprAST(double val) : Val(val) {}
    double getValue() const { return Val; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// Строковые константы
class StringExprAST : public ExprAST {
    std::string Val;
public:
    StringExprAST(const std::string& val) : Val(val) {}
    const std::string& getValue() const { return Val; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// Переменные
class VariableExprAST : public ExprAST {
    std::string Name;
public:
    VariableExprAST(const std::string& name) : Name(name) {}
    const std::string& getName() const { return Name; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// Бинарные операции
class BinaryExprAST : public ExprAST {
    int Op;
    ExprAST* LHS;
    ExprAST* RHS;
public:
    BinaryExprAST(int op, ExprAST* lhs, ExprAST* rhs) 
        : Op(op), LHS(lhs), RHS(rhs) {}
    ~BinaryExprAST() {
        delete LHS;
        delete RHS;
    }
    int getOp() const { return Op; }
    ExprAST* getLHS() const { return LHS; }
    ExprAST* getRHS() const { return RHS; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// Вызов функции
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<ExprAST*> Args;
public:
    CallExprAST(const std::string& callee, std::vector<ExprAST*> args) 
        : Callee(callee), Args(args) {}
    ~CallExprAST() {
        for (auto* arg : Args) delete arg;
    }
    const std::string& getCallee() const { return Callee; }
    const std::vector<ExprAST*>& getArgs() const { return Args; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// Объявление переменной
class VarDeclAST : public ASTNode {
    std::string Name;
    std::string Type;
    ExprAST* Init;
public:
    VarDeclAST(const std::string& name, const std::string& type, ExprAST* init = nullptr)
        : Name(name), Type(type), Init(init) {}
    ~VarDeclAST() { delete Init; }
    const std::string& getName() const { return Name; }
    const std::string& getType() const { return Type; }
    ExprAST* getInit() const { return Init; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// Присваивание
class AssignExprAST : public ExprAST {
    std::string Name;
    ExprAST* Value;
public:
    AssignExprAST(const std::string& name, ExprAST* value)
        : Name(name), Value(value) {}
    ~AssignExprAST() { delete Value; }
    const std::string& getName() const { return Name; }
    ExprAST* getValue() const { return Value; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// If оператор
class IfStmtAST : public ASTNode {
    ExprAST* Cond;
    ASTNode* Then;
    ASTNode* Else;
public:
    IfStmtAST(ExprAST* cond, ASTNode* thenStmt, ASTNode* elseStmt = nullptr)
        : Cond(cond), Then(thenStmt), Else(elseStmt) {}
    ~IfStmtAST() { delete Cond; delete Then; delete Else; }
    ExprAST* getCond() const { return Cond; }
    ASTNode* getThenBranch() const { return Then; }
    ASTNode* getElseBranch() const { return Else; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// While оператор
class WhileStmtAST : public ASTNode {
    ExprAST* Cond;
    ASTNode* Body;
public:
    WhileStmtAST(ExprAST* cond, ASTNode* body)
        : Cond(cond), Body(body) {}
    ~WhileStmtAST() { delete Cond; delete Body; }
    ExprAST* getCond() const { return Cond; }
    ASTNode* getBody() const { return Body; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// For оператор
class ForStmtAST : public ASTNode {
    VarDeclAST* Init;
    ExprAST* Cond;
    ExprAST* Inc;
    ASTNode* Body;
public:
    ForStmtAST(VarDeclAST* init, ExprAST* cond, ExprAST* inc, ASTNode* body)
        : Init(init), Cond(cond), Inc(inc), Body(body) {}
    ~ForStmtAST() { delete Init; delete Cond; delete Inc; delete Body; }
    VarDeclAST* getInit() const { return Init; }
    ExprAST* getCond() const { return Cond; }
    ExprAST* getInc() const { return Inc; }
    ASTNode* getBody() const { return Body; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// Return оператор
class ReturnStmtAST : public ASTNode {
    ExprAST* Value;
public:
    ReturnStmtAST(ExprAST* value = nullptr) : Value(value) {}
    ~ReturnStmtAST() { delete Value; }
    ExprAST* getValue() const { return Value; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// Блок операторов
class BlockStmtAST : public ASTNode {
    std::vector<ASTNode*> Statements;
public:
    void addStatement(ASTNode* stmt) {
        Statements.push_back(stmt);
    }
    ~BlockStmtAST() {
        for (auto* stmt : Statements) delete stmt;
    }
    const std::vector<ASTNode*>& getStatements() const { return Statements; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// Функция
class FunctionAST : public ASTNode {
    std::string Name;
    std::string ReturnType;
    std::vector<std::pair<std::string, std::string>> Params;
    ASTNode* Body;
public:
    FunctionAST(const std::string& name, const std::string& returnType,
                std::vector<std::pair<std::string, std::string>> params,
                ASTNode* body)
        : Name(name), ReturnType(returnType), Params(params), Body(body) {}
    ~FunctionAST() { delete Body; }
    const std::string& getName() const { return Name; }
    const std::string& getReturnType() const { return ReturnType; }
    const std::vector<std::pair<std::string, std::string>>& getParams() const { return Params; }
    ASTNode* getBody() const { return Body; }
    void codegen() override;
    void print(int depth = 0) const override;
};

// Программа
class ProgramAST : public ASTNode {
    std::vector<FunctionAST*> Functions;
public:
    void addFunction(FunctionAST* func) {
        Functions.push_back(func);
    }
    ~ProgramAST() {
        for (auto* func : Functions) delete func;
    }
    const std::vector<FunctionAST*>& getFunctions() const { return Functions; }
    void codegen() override;
    void print(int depth = 0) const override;
};

#endif // AST_H