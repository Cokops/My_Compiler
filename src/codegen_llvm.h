#ifndef CODEGEN_LLVM_H
#define CODEGEN_LLVM_H

#include "ast.h"
#include <memory>
#include <string>
#include <map>
#include <vector>

// LLVM заголовки
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"

// Класс генератора кода LLVM
class CodegenLLVM {
public:
    CodegenLLVM();
    ~CodegenLLVM();
    
    // Сгенерировать код для программы
    bool generate(ProgramAST* program, const std::string& outputFile);
    
    // Сгенерировать код для функции
    llvm::Function* generateFunction(FunctionAST* func);
    
    // Сгенерировать код для оператора
    llvm::Value* generateStatement(ASTNode* stmt);
    
    // Сгенерировать код для выражения
    llvm::Value* generateExpr(ExprAST* expr);
    
private:
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    
    // Таблица функций
    std::map<std::string, llvm::Function*> functions;
    
    // Таблица переменных
    std::map<std::string, llvm::Value*> variables;
    
    // Текущая функция
    llvm::Function* currentFunction;
    
    // Генерировать тип LLVM по имени типа
    llvm::Type* getLLVMType(const std::string& type);
    
    // Создать новую функцию
    llvm::Function* createFunction(const std::string& name, llvm::Type* returnType,
                                   const std::vector<std::pair<std::string, std::string>>& params);
    
    // Генерировать оператор if
    llvm::Value* generateIf(IfStmtAST* ifStmt);
    
    // Генерировать оператор while
    llvm::Value* generateWhile(WhileStmtAST* whileStmt);
    
    // Генерировать оператор for
    llvm::Value* generateFor(ForStmtAST* forStmt);
    
    // Генерировать оператор return
    llvm::Value* generateReturn(ReturnStmtAST* returnStmt);
    
    // Генерировать блок
    llvm::Value* generateBlock(BlockStmtAST* block);
    
    // Генерировать бинарную операцию
    llvm::Value* generateBinaryExpr(BinaryExprAST* expr);
    
    // Генерировать вызов функции
    llvm::Value* generateCall(CallExprAST* call);
};

#endif // CODEGEN_LLVM_H
