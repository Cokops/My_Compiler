#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

// Базовый класс для всех узлов AST
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int depth = 0) const = 0;
};

// Узел для выражений
class ExprAST : public ASTNode {
public:
    virtual ~ExprAST() = default;
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "ExprAST\n";
    }
protected:
    static void indent(int depth) {
        for (int i = 0; i < depth; ++i) std::cout << "  ";
    }
};

// Узел для числовых констант
class NumberExprAST : public ExprAST {
    double Val;
public:
    NumberExprAST(double val) : Val(val) {}
    double getValue() const { return Val; }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "NumberExpr: " << Val << "\n";
    }
};

// Узел для строковых констант
class StringExprAST : public ExprAST {
    std::string Val;
public:
    StringExprAST(const std::string& val) : Val(val) {}
    const std::string& getValue() const { return Val; }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "StringExpr: \"" << Val << "\"\n";
    }
};

// Узел для переменных
class VariableExprAST : public ExprAST {
    std::string Name;
public:
    VariableExprAST(const std::string& name) : Name(name) {}
    const std::string& getName() const { return Name; }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "VariableExpr: " << Name << "\n";
    }
};

// Узел для бинарных операций
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : Op(op), LHS(std::move(lhs)), RHS(std::move(rhs)) {}
    char getOp() const { return Op; }
    const ExprAST& getLHS() const { return *LHS; }
    const ExprAST& getRHS() const { return *RHS; }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "BinaryExpr: " << Op << "\n";
        LHS->print(depth + 1);
        RHS->print(depth + 1);
    }
};

// Узел для вызова функций
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args)
        : Callee(callee), Args(std::move(args)) {}
    const std::string& getCallee() const { return Callee; }
    const std::vector<std::unique_ptr<ExprAST>>& getArgs() const { return Args; }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "CallExpr: " << Callee << "\n";
        for (const auto& arg : Args) {
            arg->print(depth + 1);
        }
    }
};

// Узел для объявления переменных
class VarDeclAST : public ASTNode {
    std::string Name;
    std::string Type;
    std::unique_ptr<ExprAST> Init;
public:
    VarDeclAST(const std::string& name, const std::string& type, std::unique_ptr<ExprAST> init = nullptr)
        : Name(name), Type(type), Init(std::move(init)) {}
    const std::string& getName() const { return Name; }
    const std::string& getType() const { return Type; }
    const ExprAST* getInit() const { return Init.get(); }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "VarDecl: " << Type << " " << Name;
        if (Init) {
            std::cout << " = ";
            Init->print(depth);
        }
        std::cout << "\n";
    }
};

// Узел для присваивания
class AssignExprAST : public ExprAST {
    std::string Name;
    std::unique_ptr<ExprAST> Value;
public:
    AssignExprAST(const std::string& name, std::unique_ptr<ExprAST> value)
        : Name(name), Value(std::move(value)) {}
    const std::string& getName() const { return Name; }
    const ExprAST& getValue() const { return *Value; }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "AssignExpr: " << Name << " =\n";
        Value->print(depth + 1);
    }
};

// Узел для оператора if
class IfStmtAST : public ASTNode {
    std::unique_ptr<ExprAST> Cond;
    std::unique_ptr<ASTNode> Then Branch, ElseBranch;
public:
    IfStmtAST(std::unique_ptr<ExprAST> cond, std::unique_ptr<ASTNode> thenBr, std::unique_ptr<ASTNode> elseBr = nullptr)
        : Cond(std::move(cond)), ThenBranch(std::move(thenBr)), ElseBranch(std::move(elseBr)) {}
    const ExprAST& getCond() const { return *Cond; }
    const ASTNode* getThenBranch() const { return ThenBranch.get(); }
    const ASTNode* getElseBranch() const { return ElseBranch.get(); }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "IfStmt:\n";
        Cond->print(depth + 1);
        indent(depth + 1);
        std::cout << "Then:\n";
        ThenBranch->print(depth + 2);
        if (ElseBranch) {
            indent(depth + 1);
            std::cout << "Else:\n";
            ElseBranch->print(depth + 2);
        }
    }
};

// Узел для цикла while
class WhileStmtAST : public ASTNode {
    std::unique_ptr<ExprAST> Cond;
    std::unique_ptr<ASTNode> Body;
public:
    WhileStmtAST(std::unique_ptr<ExprAST> cond, std::unique_ptr<ASTNode> body)
        : Cond(std::move(cond)), Body(std::move(body)) {}
    const ExprAST& getCond() const { return *Cond; }
    const ASTNode& getBody() const { return *Body; }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "WhileStmt:\n";
        Cond->print(depth + 1);
        indent(depth + 1);
        std::cout << "Body:\n";
        Body->print(depth + 2);
    }
};

// Узел для цикла for
class ForStmtAST : public ASTNode {
    std::unique_ptr<VarDeclAST> Init;
    std::unique_ptr<ExprAST> Cond;
    std::unique_ptr<ExprAST> Inc;
    std::unique_ptr<ASTNode> Body;
public:
    ForStmtAST(std::unique_ptr<VarDeclAST> init, std::unique_ptr<ExprAST> cond,
               std::unique_ptr<ExprAST> inc, std::unique_ptr<ASTNode> body)
        : Init(std::move(init)), Cond(std::move(cond)), Inc(std::move(inc)), Body(std::move(body)) {}
    const VarDeclAST* getInit() const { return Init.get(); }
    const ExprAST& getCond() const { return *Cond; }
    const ExprAST& getInc() const { return *Inc; }
    const ASTNode& getBody() const { return *Body; }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "ForStmt:\n";
        if (Init) Init->print(depth + 1);
        indent(depth + 1);
        std::cout << "Condition:\n";
        Cond->print(depth + 2);
        indent(depth + 1);
        std::cout << "Increment:\n";
        Inc->print(depth + 2);
        indent(depth + 1);
        std::cout << "Body:\n";
        Body->print(depth + 2);
    }
};

// Узел для функции
class FunctionAST : public ASTNode {
    std::string Name;
    std::string ReturnType;
    std::vector<std::pair<std::string, std::string>> Params; // name, type
    std::unique_ptr<ASTNode> Body;
public:
    FunctionAST(const std::string& name, const std::string& returnType,
                std::vector<std::pair<std::string, std::string>> params,
                std::unique_ptr<ASTNode> body)
        : Name(name), ReturnType(returnType), Params(std::move(params)), Body(std::move(body)) {}
    const std::string& getName() const { return Name; }
    const std::string& getReturnType() const { return ReturnType; }
    const std::vector<std::pair<std::string, std::string>>& getParams() const { return Params; }
    const ASTNode& getBody() const { return *Body; }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "Function: " << ReturnType << " " << Name << "(\n";
        for (const auto& param : Params) {
            indent(depth + 1);
            std::cout << param.second << " " << param.first << "\n";
        }
        indent(depth + 1);
        std::cout << ")\n";
        Body->print(depth + 1);
    }
};

// Узел для оператора return
class ReturnStmtAST : public ASTNode {
    std::unique_ptr<ExprAST> Value;
public:
    ReturnStmtAST(std::unique_ptr<ExprAST> value = nullptr)
        : Value(std::move(value)) {}
    const ExprAST* getValue() const { return Value.get(); }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "ReturnStmt";
        if (Value) {
            std::cout << ":\n";
            Value->print(depth + 1);
        } else {
            std::cout << "\n";
        }
    }
};

// Узел для блока операторов
class BlockStmtAST : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> Statements;
public:
    void addStatement(std::unique_ptr<ASTNode> stmt) {
        Statements.push_back(std::move(stmt));
    }
    const std::vector<std::unique_ptr<ASTNode>>& getStatements() const { return Statements; }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "BlockStmt:\n";
        for (const auto& stmt : Statements) {
            stmt->print(depth + 1);
        }
    }
};

// Узел для программы (содержит все функции)
class ProgramAST : public ASTNode {
    std::vector<std::unique_ptr<FunctionAST>> Functions;
public:
    void addFunction(std::unique_ptr<FunctionAST> func) {
        Functions.push_back(std::move(func));
    }
    const std::vector<std::unique_ptr<FunctionAST>>& getFunctions() const { return Functions; }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "Program:\n";
        for (const auto& func : Functions) {
            func->print(depth + 1);
        }
    }
};

#endif // AST_H
