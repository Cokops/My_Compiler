#include <iostream>
#include <cstdlib>
#include <string>
#include <windows.h>
#include <cstring>

std::string getExePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string path(buffer);
    size_t pos = path.find_last_of("\\");
    return path.substr(0, pos);
}

int main() {
    std::string exeDir = getExePath();
    std::string releaseDir = exeDir + "\\..\\build\\Release";
    std::string testFile = exeDir + "\\text.txt";
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "       MY COMPILER RUNNER" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::cout << "[1] COMPILING..." << std::endl;
    std::string copyCmd = "copy \"" + testFile + "\" \"" + releaseDir + "\\text.txt\" > nul";
    system(copyCmd.c_str());
    
    std::string compileCmd = "cd /d \"" + releaseDir + "\" && my_compiler.exe text.txt";
    system(compileCmd.c_str());
    std::cout << std::endl;
    
    std::cout << "[2] LLVM IR CODE:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::string typeCmd = "type \"" + releaseDir + "\\output.ll\"";
    system(typeCmd.c_str());
    std::cout << "----------------------------------------" << std::endl;
    std::cout << std::endl;
    
    std::cout << "[3] EXECUTING..." << std::endl;
    std::string lliCmd = "cd /d \"" + releaseDir + "\" && lli output.ll";
    int result = system(lliCmd.c_str());
    std::cout << std::endl;
    
    std::cout << "[4] RESULT:" << std::endl;
    
    int exitCode = result;
    
    std::string checkFloatCmd = "findstr \"float main\" \"" + releaseDir + "\\text.txt\" > nul";
    int isFloat = system(checkFloatCmd.c_str());
    
    if (isFloat == 0) {
        float floatValue;
        memcpy(&floatValue, &exitCode, sizeof(float));
        std::cout << "Raw int: " << exitCode << std::endl;
    } else {
        std::cout << "Program returned: " << exitCode << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "            DONE!" << std::endl;
    std::cout << "========================================" << std::endl;
    system("pause");
    
    return 0;
}