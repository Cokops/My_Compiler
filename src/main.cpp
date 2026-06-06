#include <iostream>
#include <fstream>
#include <string>
#include "ast.h"
#include "codegen_llvm.h"
#include "parser.hpp"

// Глобальные переменные для Flex
extern FILE* yyin;
extern int yyparse();
extern int yylineno;

ProgramAST* g_program = nullptr;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> [-o <output_file>]\n";
        return 1;
    }
    
    // Открываем входной файл
    FILE* inputFile = fopen(argv[1], "r");
    if (!inputFile) {
        std::cerr << "Error: Cannot open input file '" << argv[1] << "'\n";
        return 1;
    }
    
    // Определяем выходной файл
    std::string outputFile = "output.ll";
    for (int i = 2; i < argc; i++) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            outputFile = argv[i + 1];
            i++;
        }
    }
    
    // Настраиваем Flex
    yyin = inputFile;
    yylineno = 1;
    
    // Создаем AST
    g_program = new ProgramAST();
    
    // Запускаем парсер
    int parseResult = yyparse();
    
    if (parseResult != 0) {
        std::cerr << "Parse failed!\n";
        delete g_program;
        fclose(inputFile);
        return 1;
    }
    
    // Выводим AST для отладки
    std::cout << "AST:\n";
    g_program->print();
    
    // Генерация LLVM IR
    CodegenLLVM codegen;
    if (codegen.generate(g_program, outputFile)) {
        std::cout << "LLVM IR generated successfully: " << outputFile << "\n";
    } else {
        std::cerr << "Failed to generate LLVM IR\n";
        delete g_program;
        fclose(inputFile);
        return 1;
    }
    
    // Очистка
    delete g_program;
    fclose(inputFile);
    
    return 0; 
}