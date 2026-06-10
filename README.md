# Компилятор для учебного императивного языка программирования

## Описание проекта

Это компилятор для простого императивного языка программирования, реализованный на C++ с использованием:
- **Flex** — лексический анализ
- **Bison** — синтаксический анализ
- **LLVM** — генерация и оптимизация кода

## Поддерживаемые возможности

### Базовые типы
- `int` — целые числа
- `float` — числа с плавающей точкой
- `bool` — логические значения
- `void` — отсутствие значения

### Управляющие структуры
- Условные операторы: `if`, `if-else`
- Циклы: `while`, `for`
- Блоки кода: `{ ... }`

### Функции
- Объявление функций с параметрами
- Возвращаемые значения
- Рекурсивные вызовы

### Операторы
- Арифметические: `+`, `-`, `*`, `/`
- Сравнения: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Логические: `&&`, `||`, `!`
- Присваивание: `=`

## Архитектура компилятора

```
source.code
↓
[Lexer / Scanner] — Flex
↓
[Tokens stream]
↓
[Parser] — Bison
↓
[AST (Abstract Syntax Tree)]
↓
[LLVM IR Generator]
↓
[LLVM IR code]
```

## Структура проекта

```
My_Compiler/
├── CMakeLists.txt            # Основной скрипт сборки
├── Dockerfile                # Контейнеризация
├── README.md
├── src/
│   ├── ast.h / ast.cpp       # Абстрактное синтаксическое дерево (AST)
│   ├── scanner.l             # Грамматика для Flex (лексер)
│   ├── parser.y              # Грамматика для Bison (парсер)
│   ├── codegen_llvm.h/cpp    # Генератор LLVM IR
│   └── main.cpp              # Точка входа (аргументы CLI)
├── tests/                    # Автоматические тесты (C++ утилита)
└── work/                     # Скрипты для локального запуска
```

## Зависимости

- C++17 или выше
- CMake 3.16 или выше
- Flex
- Bison
- LLVM 14 или выше

## Сборка

### Установка зависимостей (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential flex bison cmake
sudo apt install llvm-14-dev clang-14
```
### Генерация лексера и парсера

# Генерация лексера из scanner.l
flex -o lexer.cpp scanner.l

# Генерация парсера из parser.y
bison -d -o parser.cpp parser.y

### Установка зависимостей (Windows с vcpkg)

```bash
# Установка vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# Установка зависимостей
./vcpkg install flex:b32-windows bison:b32-windows llvm:x64-windows
```

### Сборка на Windows (Visual Studio)

```bash
mkdir build
cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

## Использование

### Базовые команды

# Компиляция программы в LLVM IR
./build/Release/my_compiler.exe input.txt -o output.ll

# Или с указанием полного пути
./my_compiler.exe text.txt

# Просмотр сгенерированного LLVM IR
cat output.ll

# Запуск IR через интерпретатор LLVM
lli output.ll

# Проверка кода возврата
Write-Host "Result: $LASTEXITCODE"

## Лицензия

Проект выполнен в рамках учебного задания.

## Автор

**Cokops**
- GitHub: [@Cokops](https://github.com/Cokops)
- Email: gerasev-z@mail.ru
---
