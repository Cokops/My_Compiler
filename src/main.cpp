#include <iostream>
#include <fstream>
#include <memory>
#include "ast.h"
#include "scanner.h"
#include "codegen_llvm.h"
#include "parser.hpp" // Bison генерирует parser.hpp

// Глобальные переменные
Scanner* g_scanner = nullptr;
ProgramAST* g_program = nullptr;

// Прототипы функций
int yyparse();

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> [-o <output_file>]\n";
        return 1;
    }
    
    // Открываем входной файл
    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
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
    
    // Создаем лексический анализатор
    g_scanner = new Scanner(inputFile);
    
    // Создаем AST
    g_program = new ProgramAST();
    
    // Запускаем парсер
    int parseResult = yyparse();
    
    if (parseResult != 0) {
        std::cerr << "Parse failed!\n";
        delete g_scanner;
        delete g_program;
        return 1;
    }
    
    // Выводим AST для отладки
    std::cout << "AST:\n";
    g_program->print();
    
    // Семантический анализ
    // TODO: Запустить семантический анализатор
    
    // Генерация LLVM IR
    CodegenLLVM codegen;
    if (codegen.generate(g_program, outputFile)) {
        std::cout << "LLVM IR generated successfully: " << outputFile << "\n";
    } else {
        std::cerr << "Failed to generate LLVM IR\n";
        delete g_scanner;
        delete g_program;
        return 1;
    }
    
    // Очистка
    delete g_scanner;
    delete g_program;
    
    return 0;
}
