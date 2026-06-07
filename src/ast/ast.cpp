#include "ast.h"
#include <memory>
#include <vector>
#include <map>
#include <iostream>

static void indent(int depth) {
    for (int i = 0; i < depth; ++i) std::cout << "  ";
}

void ASTNode::print(int depth) const {
    indent(depth);
    std::cout << "ASTNode\n";
}

void ExprAST::print(int depth) const {
    indent(depth);
    std::cout << "ExprAST\n";
}

void NumberExprAST::print(int depth) const {
    indent(depth);
    if (isFloat) {
        std::cout << "FloatExpr: " << Val << "\n";
    } else {
        std::cout << "NumberExpr: " << (int)Val << "\n";
    }
}

void NumberExprAST::codegen() {
    std::cout << "  Load number: " << Val << (isFloat ? " (float)" : " (int)") << "\n";
}

void StringExprAST::print(int depth) const {
    indent(depth);
    std::cout << "StringExpr: \"" << Val << "\"\n";
}

void StringExprAST::codegen() {
    std::cout << "  Load string: \"" << Val << "\"\n";
}

void BoolExprAST::print(int depth) const {
    indent(depth);
    std::cout << "BoolExpr: " << (Val ? "true" : "false") << "\n";
}

void BoolExprAST::codegen() {
    std::cout << "  Load bool: " << (Val ? "true" : "false") << "\n";
}

void CharExprAST::print(int depth) const {
    indent(depth);
    std::cout << "CharExpr: '" << Val << "'\n";
}

void CharExprAST::codegen() {
    std::cout << "  Load char: '" << Val << "' (" << (int)Val << ")\n";
}

void VariableExprAST::print(int depth) const {
    indent(depth);
    std::cout << "VariableExpr: " << Name << "\n";
}

void VariableExprAST::codegen() {
    std::cout << "  Load variable: " << Name << "\n";
}

void BinaryExprAST::print(int depth) const {
    indent(depth);
    std::cout << "BinaryExpr: " << Op << "\n";
    if (LHS) LHS->print(depth + 1);
    if (RHS) RHS->print(depth + 1);
}

void BinaryExprAST::codegen() {
    std::cout << "  Binary op: " << Op << "\n";
    if (LHS) LHS->codegen();
    if (RHS) RHS->codegen();
}

void CallExprAST::print(int depth) const {
    indent(depth);
    std::cout << "CallExpr: " << Callee << "\n";
    for (const auto& arg : Args) {
        if (arg) arg->print(depth + 1);
    }
}

void CallExprAST::codegen() {
    std::cout << "  Call function: " << Callee << "\n";
    for (auto* arg : Args) {
        if (arg) arg->codegen();
    }
}

void VarDeclAST::print(int depth) const {
    indent(depth);
    std::cout << "VarDecl: " << Type << " " << Name;
    if (Init) {
        std::cout << " = ";
        Init->print(0);
    } else {
        std::cout << "\n";
    }
}

void VarDeclAST::codegen() {
    std::cout << "Declare variable: " << Name << " : " << Type << "\n";
    if (Init) Init->codegen();
}

void AssignExprAST::print(int depth) const {
    indent(depth);
    std::cout << "AssignExpr: " << Name << " =\n";
    if (Value) Value->print(depth + 1);
}

void AssignExprAST::codegen() {
    std::cout << "Assign to: " << Name << "\n";
    if (Value) Value->codegen();
}

void IfStmtAST::print(int depth) const {
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

void IfStmtAST::codegen() {
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

void WhileStmtAST::print(int depth) const {
    indent(depth);
    std::cout << "WhileStmt:\n";
    if (Cond) Cond->print(depth + 1);
    indent(depth + 1);
    std::cout << "Body:\n";
    if (Body) Body->print(depth + 2);
}

void WhileStmtAST::codegen() {
    std::cout << "While loop\n";
    std::cout << "  Condition:\n";
    if (Cond) Cond->codegen();
    std::cout << "  Body:\n";
    if (Body) Body->codegen();
}

void ForStmtAST::print(int depth) const {
    indent(depth);
    std::cout << "ForStmt:\n";
    if (Init) Init->print(depth + 1);
    indent(depth + 1);
    std::cout << "Condition:\n";
    if (Cond) Cond->print(depth + 2);
    indent(depth + 1);
    std::cout << "Increment:\n";
    if (Inc) Inc->print(depth + 2);
    indent(depth + 1);
    std::cout << "Body:\n";
    if (Body) Body->print(depth + 2);
}

void ForStmtAST::codegen() {
    std::cout << "For loop\n";
    if (Init) Init->codegen();
    std::cout << "  Condition:\n";
    if (Cond) Cond->codegen();
    std::cout << "  Increment:\n";
    if (Inc) Inc->codegen();
    std::cout << "  Body:\n";
    if (Body) Body->codegen();
}

void ReturnStmtAST::print(int depth) const {
    indent(depth);
    std::cout << "ReturnStmt";
    if (Value) {
        std::cout << ":\n";
        Value->print(depth + 1);
    } else {
        std::cout << "\n";
    }
}

void ReturnStmtAST::codegen() {
    std::cout << "Return";
    if (Value) {
        std::cout << ":\n";
        Value->codegen();
    } else {
        std::cout << " void\n";
    }
}

void BlockStmtAST::print(int depth) const {
    indent(depth);
    std::cout << "BlockStmt:\n";
    for (const auto& stmt : Statements) {
        if (stmt) stmt->print(depth + 1);
    }
}

void BlockStmtAST::codegen() {
    for (auto* stmt : Statements) {
        if (stmt) stmt->codegen();
    }
}

void FunctionAST::print(int depth) const {
    indent(depth);
    std::cout << "Function: " << ReturnType << " " << Name << "(\n";
    for (const auto& param : Params) {
        indent(depth + 1);
        std::cout << param.second << " " << param.first << "\n";
    }
    indent(depth + 1);
    std::cout << ")\n";
    if (Body) Body->print(depth + 1);
}

void FunctionAST::codegen() {
    std::cout << "Function: " << Name << "\n";
    if (Body) Body->codegen();
}

void ProgramAST::print(int depth) const {
    indent(depth);
    std::cout << "Program:\n";
    for (const auto& func : Functions) {
        if (func) func->print(depth + 1);
    }
}

void ProgramAST::codegen() {
    for (auto* func : Functions) {
        if (func) func->codegen();
    }
}