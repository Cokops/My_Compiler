#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>

// Базовый класс для всех узлов AST
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void codegen() {
        std::cout << "Generating code for AST node\n";
    }
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
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "ExprAST\n";
    }
};

// Числовые константы
class NumberExprAST : public ExprAST {
    double Val;
public:
    NumberExprAST(double val) : Val(val) {}
    double getValue() const { return Val; }  // ← ДОБАВЛЕНО
    void codegen() override {
        std::cout << "  Load number: " << Val << "\n";
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "NumberExpr: " << Val << "\n";
    }
};

// Строковые константы
class StringExprAST : public ExprAST {
    std::string Val;
public:
    StringExprAST(const std::string& val) : Val(val) {}
    const std::string& getValue() const { return Val; }
    void codegen() override {
        std::cout << "  Load string: \"" << Val << "\"\n";
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "StringExpr: \"" << Val << "\"\n";
    }
};

// Переменные
class VariableExprAST : public ExprAST {
    std::string Name;
public:
    VariableExprAST(const std::string& name) : Name(name) {}
    const std::string& getName() const { return Name; }  // ← ДОБАВЛЕНО
    void codegen() override {
        std::cout << "  Load variable: " << Name << "\n";
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "VariableExpr: " << Name << "\n";
    }
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
    int getOp() const { return Op; }           // ← ДОБАВЛЕНО
    ExprAST* getLHS() const { return LHS; }    // ← ДОБАВЛЕНО
    ExprAST* getRHS() const { return RHS; }    // ← ДОБАВЛЕНО
    void codegen() override {
        std::cout << "  Binary op: " << Op << "\n";
        if (LHS) LHS->codegen();
        if (RHS) RHS->codegen();
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "BinaryExpr: " << Op << "\n";
        if (LHS) LHS->print(depth + 1);
        if (RHS) RHS->print(depth + 1);
    }
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
    const std::string& getCallee() const { return Callee; }           // ← ДОБАВЛЕНО
    const std::vector<ExprAST*>& getArgs() const { return Args; }     // ← ДОБАВЛЕНО
    void codegen() override {
        std::cout << "  Call function: " << Callee << "\n";
        for (auto* arg : Args) {
            if (arg) arg->codegen();
        }
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "CallExpr: " << Callee << "\n";
        for (auto* arg : Args) {
            arg->print(depth + 1);
        }
    }
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
    const std::string& getName() const { return Name; }   // ← ДОБАВЛЕНО
    const std::string& getType() const { return Type; }   // ← ДОБАВЛЕНО
    ExprAST* getInit() const { return Init; }             // ← ДОБАВЛЕНО
    void codegen() override {
        std::cout << "Declare variable: " << Name << " : " << Type << "\n";
        if (Init) Init->codegen();
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "VarDecl: " << Type << " " << Name;
        if (Init) {
            std::cout << " = ";
            Init->print(0);
        }
        std::cout << "\n";
    }
};

// Присваивание
class AssignExprAST : public ExprAST {
    std::string Name;
    ExprAST* Value;
public:
    AssignExprAST(const std::string& name, ExprAST* value)
        : Name(name), Value(value) {}
    ~AssignExprAST() { delete Value; }
    const std::string& getName() const { return Name; }   // ← ДОБАВЛЕНО
    ExprAST* getValue() const { return Value; }           // ← ДОБАВЛЕНО
    void codegen() override {
        std::cout << "Assign to: " << Name << "\n";
        if (Value) Value->codegen();
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "AssignExpr: " << Name << " =\n";
        if (Value) Value->print(depth + 1);
    }
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
    ExprAST* getCond() const { return Cond; }               // ← ДОБАВЛЕНО
    ASTNode* getThenBranch() const { return Then; }         // ← ДОБАВЛЕНО
    ASTNode* getElseBranch() const { return Else; }         // ← ДОБАВЛЕНО
    void codegen() override {
        std::cout << "If statement\n";
        std::cout << "  Condition:\n";
        if (Cond) Cond->codegen();
        std::cout << "  Then branch:\n";
        if (Then) Then->codegen();
        if (Else) {
            std::cout << "  Else branch:\n";
            Else->codegen();
        }
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "IfStmt:\n";
        if (Cond) Cond->print(depth + 1);
        indent(depth + 1);
        std::cout << "Then:\n";
        if (Then) Then->print(depth + 2);
        if (Else) {
            indent(depth + 1);
            std::cout << "Else:\n";
            Else->print(depth + 2);
        }
    }
};

// While оператор
class WhileStmtAST : public ASTNode {
    ExprAST* Cond;
    ASTNode* Body;
public:
    WhileStmtAST(ExprAST* cond, ASTNode* body)
        : Cond(cond), Body(body) {}
    ~WhileStmtAST() { delete Cond; delete Body; }
    ExprAST* getCond() const { return Cond; }   // ← ДОБАВЛЕНО
    ASTNode* getBody() const { return Body; }   // ← ДОБАВЛЕНО
    void codegen() override {
        std::cout << "While loop\n";
        std::cout << "  Condition:\n";
        if (Cond) Cond->codegen();
        std::cout << "  Body:\n";
        if (Body) Body->codegen();
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "WhileStmt:\n";
        if (Cond) Cond->print(depth + 1);
        indent(depth + 1);
        std::cout << "Body:\n";
        if (Body) Body->print(depth + 2);
    }
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
    VarDeclAST* getInit() const { return Init; }   // ← ДОБАВЛЕНО
    ExprAST* getCond() const { return Cond; }      // ← ДОБАВЛЕНО
    ExprAST* getInc() const { return Inc; }        // ← ДОБАВЛЕНО
    ASTNode* getBody() const { return Body; }      // ← ДОБАВЛЕНО
    void codegen() override {
        std::cout << "For loop\n";
        if (Init) Init->codegen();
        std::cout << "  Condition:\n";
        if (Cond) Cond->codegen();
        std::cout << "  Increment:\n";
        if (Inc) Inc->codegen();
        std::cout << "  Body:\n";
        if (Body) Body->codegen();
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "ForStmt:\n";
        if (Init) Init->print(depth + 1);
        if (Cond) Cond->print(depth + 1);
        if (Inc) Inc->print(depth + 1);
        if (Body) Body->print(depth + 1);
    }
};

// Return оператор
class ReturnStmtAST : public ASTNode {
    ExprAST* Value;
public:
    ReturnStmtAST(ExprAST* value = nullptr) : Value(value) {}
    ~ReturnStmtAST() { delete Value; }
    ExprAST* getValue() const { return Value; }   // ← ДОБАВЛЕНО
    void codegen() override {
        std::cout << "Return";
        if (Value) {
            std::cout << ":\n";
            Value->codegen();
        } else {
            std::cout << " void\n";
        }
    }
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
    const std::vector<ASTNode*>& getStatements() const { return Statements; }  // ← ДОБАВЛЕНО
    void codegen() override {
        for (auto* stmt : Statements) {
            if (stmt) stmt->codegen();
        }
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "BlockStmt:\n";
        for (auto* stmt : Statements) {
            stmt->print(depth + 1);
        }
    }
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
    const std::string& getName() const { return Name; }                       // ← ДОБАВЛЕНО
    const std::string& getReturnType() const { return ReturnType; }           // ← ДОБАВЛЕНО
    const std::vector<std::pair<std::string, std::string>>& getParams() const { return Params; }  // ← ДОБАВЛЕНО
    ASTNode* getBody() const { return Body; }                                 // ← ДОБАВЛЕНО
    void codegen() override {
        std::cout << "Function: " << Name << "\n";
        if (Body) Body->codegen();
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "Function: " << ReturnType << " " << Name << "(\n";
        for (const auto& p : Params) {
            indent(depth + 1);
            std::cout << p.second << " " << p.first << "\n";
        }
        indent(depth + 1);
        std::cout << ")\n";
        if (Body) Body->print(depth + 1);
    }
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
    const std::vector<FunctionAST*>& getFunctions() const { return Functions; }  // ← ДОБАВЛЕНО
    void codegen() override {
        for (auto* func : Functions) {
            if (func) func->codegen();
        }
    }
    void print(int depth = 0) const override {
        indent(depth);
        std::cout << "Program:\n";
        for (auto* func : Functions) {
            func->print(depth + 1);
        }
    }
};

#endif // AST_H