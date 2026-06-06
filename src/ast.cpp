#include "ast.h"
#include <memory>
#include <vector>
#include <map>
#include <iostream>

// Реализация базовых методов AST
void ASTNode::print(int depth) const {
    indent(depth);
    std::cout << "ASTNode\n";
}

// Реализация методов ExprAST
void ExprAST::print(int depth) const {
    indent(depth);
    std::cout << "ExprAST\n";
}

// Реализация методов NumberExprAST
void NumberExprAST::print(int depth) const {
    indent(depth);
    std::cout << "NumberExpr: " << Val << "\n";
}

// Реализация методов StringExprAST
void StringExprAST::print(int depth) const {
    indent(depth);
    std::cout << "StringExpr: \"" << Val << "\"\n";
}

// Реализация методов VariableExprAST
void VariableExprAST::print(int depth) const {
    indent(depth);
    std::cout << "VariableExpr: " << Name << "\n";
}

// Реализация методов BinaryExprAST
void BinaryExprAST::print(int depth) const {
    indent(depth);
    std::cout << "BinaryExpr: " << Op << "\n";
    LHS->print(depth + 1);
    RHS->print(depth + 1);
}

// Реализация методов CallExprAST
void CallExprAST::print(int depth) const {
    indent(depth);
    std::cout << "CallExpr: " << Callee << "\n";
    for (const auto& arg : Args) {
        arg->print(depth + 1);
    }
}

// Реализация методов VarDeclAST
void VarDeclAST::print(int depth) const {
    indent(depth);
    std::cout << "VarDecl: " << Type << " " << Name;
    if (Init) {
        std::cout << " = ";
        Init->print(depth);
    }
    std::cout << "\n";
}

// Реализация методов AssignExprAST
void AssignExprAST::print(int depth) const {
    indent(depth);
    std::cout << "AssignExpr: " << Name << " =\n";
    Value->print(depth + 1);
}

// Реализация методов IfStmtAST
void IfStmtAST::print(int depth) const {
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

// Реализация методов WhileStmtAST
void WhileStmtAST::print(int depth) const {
    indent(depth);
    std::cout << "WhileStmt:\n";
    Cond->print(depth + 1);
    indent(depth + 1);
    std::cout << "Body:\n";
    Body->print(depth + 2);
}

// Реализа��ия методов ForStmtAST
void ForStmtAST::print(int depth) const {
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

// Реализация методов FunctionAST
void FunctionAST::print(int depth) const {
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

// Реализация методов ReturnStmtAST
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

// Реализация методов BlockStmtAST
void BlockStmtAST::print(int depth) const {
    indent(depth);
    std::cout << "BlockStmt:\n";
    for (const auto& stmt : Statements) {
        stmt->print(depth + 1);
    }
}

// Реализация методов ProgramAST
void ProgramAST::print(int depth) const {
    indent(depth);
    std::cout << "Program:\n";
    for (const auto& func : Functions) {
        func->print(depth + 1);
    }
}
