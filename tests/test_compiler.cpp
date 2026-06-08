#include <iostream>
#include <cstdlib>
#include <string>
#include <windows.h>
#include <cstring>
#include <vector>
#include <fstream>

std::string getExePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string path(buffer);
    size_t pos = path.find_last_of("\\");
    return path.substr(0, pos);
}

struct Test {
    std::string name;
    std::string code;
    int expected;
};

int main() {
    std::string exeDir = getExePath();
    std::string releaseDir = exeDir + "\\..\\build\\Release";
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "       COMPILER TEST SUITE" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
std::vector<Test> tests = {
    // INT
    {"INT - Addition", "int main() { return 10 + 20; }", 30},
    {"INT - Subtraction", "int main() { return 50 - 30; }", 20},
    {"INT - Multiplication", "int main() { return 6 * 7; }", 42},
    {"INT - Division", "int main() { return 100 / 4; }", 25},
    {"INT - Negative", "int main() { int a = -10; return a; }", -10},
    {"INT - Multiple operations", "int main() { int a=10; int b=5; int c=3; return (a+b)*c - a/b; }", 43},
    {"INT - Large number", "int main() { return 1000 * 100; }", 100000},
    
    // FLOAT
    {"FLOAT - Multiplication", "int main() { float a=3.5; float b=2.0; int c = a * b; return c; }", 7},
    {"FLOAT - Division", "int main() { float a=10.0; float b=3.0; int c = a / b; return c; }", 3},
    {"FLOAT - Addition", "int main() { float a=1.5; float b=2.5; int c = a + b; return c; }", 4},
    {"FLOAT - Subtraction", "int main() { float a=5.5; float b=2.5; int c = a - b; return c; }", 3},
    
    // BOOL
    {"BOOL - True", "int main() { bool flag = true; return flag; }", 1},
    {"BOOL - False", "int main() { bool flag = false; return flag; }", 0},
    {"BOOL - AND true", "int main() { int a=5; int b=10; return (a>0) && (b<20); }", 1},
    {"BOOL - AND false", "int main() { int a=5; int b=10; return (a<0) && (b<20); }", 0},
    {"BOOL - OR true", "int main() { int a=0; int b=10; return (a>0) || (b<20); }", 1},
    {"BOOL - OR false", "int main() { int a=0; int b=10; return (a>0) || (b>20); }", 0},
    {"BOOL - Complex", "int main() { int a=5; int b=10; return (a>0) && (b<20) || (a==b); }", 1},
    
    // CHAR
    {"CHAR - Literal", "int main() { char c = 'A'; return c; }", 65},
    {"CHAR - Lowercase", "int main() { char c = 'a'; return c; }", 97},
    {"CHAR - Digit", "int main() { char c = '5'; return c; }", 53},
    
    // COMPARE
    {"COMPARE - EQ true", "int main() { int a=10; int b=10; return a == b; }", 1},
    {"COMPARE - EQ false", "int main() { int a=10; int b=20; return a == b; }", 0},
    {"COMPARE - NEQ false", "int main() { int a=10; int b=10; return a != b; }", 0},
    {"COMPARE - LT true", "int main() { int a=5; int b=10; return a < b; }", 1},
    {"COMPARE - LT false", "int main() { int a=10; int b=5; return a < b; }", 0},
    {"COMPARE - GT true", "int main() { int a=15; int b=10; return a > b; }", 1},
    {"COMPARE - GT false", "int main() { int a=10; int b=15; return a > b; }", 0},
    {"COMPARE - LE true", "int main() { int a=10; int b=10; return a <= b; }", 1},
    {"COMPARE - LE false", "int main() { int a=15; int b=10; return a <= b; }", 0},
    {"COMPARE - GE true", "int main() { int a=15; int b=10; return a >= b; }", 1},
    {"COMPARE - GE false", "int main() { int a=10; int b=15; return a >= b; }", 0},
    
    // IF-ELSE
    {"IF-ELSE - True", "int main() { int a=10; if(a>5){return 100;}else{return 200;} }", 100},
    {"IF-ELSE - False", "int main() { int a=3; if(a>5){return 100;}else{return 200;} }", 200},
    {"IF-ELSE - Nested", "int main() { int a=15; int b=10; if(a>10){if(b>5){return 1;}else{return 2;}}return 0; }", 1},
    {"IF-ELSE - No else", "int main() { int a=10; if(a>5){return 100;} return 0; }", 100},
    {"IF-ELSE - Multi condition", "int main() { int a=5; int b=10; if(a>b){return 1;}else if(a<b){return 2;}else{return 3;} }", 2},
    
    // WHILE
    {"WHILE - Sum 1..10", "int main() { int s=0; int i=0; while(i<=10){s=s+i;i=i+1;} return s; }", 55},
    {"WHILE - Sum even", "int main() { int s=0; int i=0; while(i<=20){s=s+i;i=i+2;} return s; }", 110},
    {"WHILE - Factorial", "int main() { int r=1; int i=1; while(i<=5){r=r*i;i=i+1;} return r; }", 120},
    {"WHILE - Count digits", "int main() { int n=12345; int c=0; while(n>0){c=c+1; n=n/10;} return c; }", 5},
    
    // FUNCTIONS
    {"FUNCTION - Simple add", "int add(int x,int y){return x+y;} int main(){return add(10,20);}", 30},
    {"FUNCTION - Subtract", "int sub(int x,int y){return x-y;} int main(){return sub(50,30);}", 20},
    {"FUNCTION - Multiply", "int mul(int x,int y){return x*y;} int main(){return mul(6,7);}", 42},
    {"FUNCTION - Divide", "int div(int x,int y){return x/y;} int main(){return div(100,4);}", 25},
    {"FUNCTION - Multiple calls", "int add(int x,int y){return x+y;} int main(){int a=add(5,3); int b=add(2,4); return a+b;}", 14},
    {"FUNCTION - Nested call", "int add(int x,int y){return x+y;} int mul(int x,int y){return x*y;} int main(){return mul(add(2,3),add(4,5));}", 45},
    {"FUNCTION - Recursive factorial", "int fact(int n){if(n<=1){return 1;}else{return n*fact(n-1);}} int main(){return fact(5);}", 120},
    {"FUNCTION - Recursive fibonacci", "int fib(int n){if(n<=1){return n;}else{return fib(n-1)+fib(n-2);}} int main(){return fib(10);}", 55},
    
    // VARIABLES
    {"VAR - Multiple declarations", "int main(){int a=5; int b=10; int c=15; return a+b+c;}", 30},
    {"VAR - Reassignment", "int main(){int a=5; a=10; return a;}", 10},
    {"VAR - Swap", "int main(){int a=5; int b=10; int t=a; a=b; b=t; return a;}", 10},
    
    // COMBINED
    {"COMBINED - Max of three", "int max2(int a,int b){if(a>b){return a;}else{return b;}} int max3(int a,int b,int c){return max2(max2(a,b),c);} int main(){return max3(50,30,40);}", 50},
    {"COMBINED - Power function", "int pow(int base,int exp){int r=1; int i=0; while(i<exp){r=r*base;i=i+1;} return r;} int main(){return pow(2,5);}", 32}
};
    
    int passed = 0;
    int total = 0;
    
    for (const auto& test : tests) {
        total++;
        std::cout << "[" << total << "] " << test.name << "... ";
        std::cout.flush();
        
        // Сохраняем в test.txt
        std::string testFile = releaseDir + "\\test.txt";
        FILE* f = fopen(testFile.c_str(), "w");
        if (!f) {
            std::cout << "CAN'T OPEN FILE" << std::endl;
            continue;
        }
        fprintf(f, "%s", test.code.c_str());
        fclose(f);
        
        // Компилируем
        std::string compileCmd = "cd /d \"" + releaseDir + "\" && my_compiler.exe test.txt > nul 2>&1";
        if (system(compileCmd.c_str()) != 0) {
            std::cout << "COMPILE ERROR" << std::endl;
            continue;
        }
        
        // Запускаем
        std::string lliCmd = "cd /d \"" + releaseDir + "\" && lli output.ll > nul 2>&1";
        system(lliCmd.c_str());
        
        // Получаем результат (исправлено!)
        int result = system(lliCmd.c_str());
        
        // Проверяем
        if (result == test.expected) {
            std::cout << "✓ PASSED (" << result << ")" << std::endl;
            passed++;
        } else {
            std::cout << "✗ FAILED (expected " << test.expected << ", got " << result << ")" << std::endl;
        }
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "       RESULTS: " << passed << " / " << total << " PASSED" << std::endl;
    std::cout << "========================================" << std::endl;
    system("pause");
    
    return 0;
}