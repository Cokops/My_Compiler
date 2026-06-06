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

// Токены операторов (должны совпадать с parser.y)
namespace Tokens {
    const int PLUS = 274;
    const int MINUS = 275;
    const int MULTIPLY = 276;
    const int DIVIDE = 277;
    const int EQ = 278;
    const int NEQ = 279;
    const int LT = 280;
    const int GT = 281;
    const int LE = 282;
    const int GE = 283;
    const int AND = 284;
    const int OR = 285;
    const int NOT = 286;
}

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
    
    // Генерировать бинарную операцию
    llvm::Value* generateBinaryExpr(BinaryExprAST* expr);
    
    // Генерировать вызов функции
    llvm::Value* generateCall(CallExprAST* call);
    
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
};

#endif // CODEGEN_LLVM_H