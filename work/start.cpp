#include <iostream>
#include <cstdlib>
#include <string>
#include <windows.h>

std::string getExePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string path(buffer);
    size_t pos = path.find_last_of("\\");
    return path.substr(0, pos);
}

int main() {
    // Получаем путь к папке, где находится start.exe (папка work)
    std::string exeDir = getExePath();
    
    // Путь к папке Release
    std::string releaseDir = exeDir + "\\..\\build\\Release";
    
    std::string testFile = exeDir + "\\text.txt";
    
    system("echo ========================================");
    system("echo        MY COMPILER RUNNER");
    system("echo ========================================");
    system("echo.");
    
    // 1. Компиляция - копируем test.txt в папку Release
    system("echo [1] COMPILING...");
    std::string copyCmd = "copy \"" + testFile + "\" \"" + releaseDir + "\\text.txt\" > nul";
    system(copyCmd.c_str());
    
    std::string compileCmd = "cd /d \"" + releaseDir + "\" && my_compiler.exe text.txt";  // ← ИСПРАВЛЕНО: test.txt
    system(compileCmd.c_str());
    system("echo.");
    
    // 2. Показ LLVM IR
    system("echo [2] LLVM IR CODE:");
    system("echo ----------------------------------------");
    std::string typeCmd = "type \"" + releaseDir + "\\output.ll\"";
    system(typeCmd.c_str());
    system("echo ----------------------------------------");
    system("echo.");
    
    // 3. Запуск
    system("echo [3] EXECUTING...");
    std::string lliCmd = "cd /d \"" + releaseDir + "\" && lli output.ll";
    int result = system(lliCmd.c_str());
    system("echo.");
    
    // 4. Результат
    system("echo [4] RESULT:");
    std::cout << "Program returned: " << result << std::endl;
    
    system("echo.");
    system("echo ========================================");
    system("echo            DONE!");
    system("echo ========================================");
    system("pause");
    
    return 0;
}