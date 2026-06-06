#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

void runCommand(const std::string& cmd) {
    std::cout << "Executing: " << cmd << std::endl;
    int result = system(cmd.c_str());
    if (result != 0) {
        std::cerr << "Command failed with code: " << result << std::endl;
    }
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return "File not found!";
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    return content;
}

int main() {
    std::string compilerDir = "C:\\Users\\Артём\\Desktop\\My_Compiler\\build\\Release";
    std::string currentDir = "C:\\Users\\Артём\\Desktop\\My_Compiler\\work";
    
    // Переходим в папку с компилятором
    std::string cdCmd = "cd /d " + compilerDir;
    system(cdCmd.c_str());
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "       MY COMPILER RUNNER (C++)" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 1. Запуск компилятора
    std::cout << "[1] RUNNING COMPILER..." << std::endl;
    system("my_compiler.exe test.txt");
    std::cout << std::endl;
    
    // 2. Показ LLVM IR кода
    std::cout << "[2] LLVM IR CODE:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::string llvmIR = readFile("output.ll");
    std::cout << llvmIR;
    std::cout << "----------------------------------------" << std::endl;
    
    // 3. Запуск LLVM интерпретатора
    std::cout << "\n[3] EXECUTING PROGRAM..." << std::endl;
    int result = system("lli output.ll");
    
    // 4. Показ результата
    std::cout << "\n[4] RESULT:" << std::endl;
    std::cout << "Program returned: " << result << std::endl;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "            DONE!" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    return 0;
}