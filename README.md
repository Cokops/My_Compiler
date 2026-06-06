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
[Lexer / Scanner] — Flex (или ручной)
    ↓
[Tokens stream]
    ↓
[Parser] — Bison (или ручной)
    ↓
[AST (Abstract Syntax Tree)]
    ↓
[Semantic Analyzer] — проверка типов, области видимости
    ↓
[LLVM IR Generator]
    ↓
[LLVM IR code]
    ↓
[LLVM Optimizer]
    ↓
[Machine Code (x86, x64, и т.д.)]
```

## Структура проекта

```
My_Compiler/
├── CMakeLists.txt          # Конфигурация сборки
├── Makefile               # Упрощённый Makefile
├── src/
│   ├── ast.h              # Определения AST
│   ├── ast.cpp            # Реализация AST
│   ├── scanner.h          # Интерфейс лексического анализатора
│   ├── scanner.cpp        # Реализация лексического анализатора
│   ├── scanner.l          # Грамматика для Flex
│   ├── parser.y           # Грамматика для Bison
│   ├── semantic_analyzer.h # Интерфейс семантического анализатора
│   ├── semantic_analyzer.cpp # Реализация семантического анализатора
│   ├── codegen_llvm.h     # Интерфейс генератора LLVM
│   ├── codegen_llvm.cpp   # Реализация генератора LLVM
│   └── main.cpp           # Точка входа
├── example.myc            # Пример программы
└── README.md              # Документация
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

### Установка зависимостей (Windows с vcpkg)

```bash
# Установка vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# Установка зависимостей
./vcpkg install flex:b32-windows bison:b32-windows llvm:x64-windows
```

### Сборка с помощью Makefile

```bash
make build
```

### Сборка с помощью CMake

```bash
# Создание директории сборки
mkdir build
cd build

# Настройка CMake
cmake ..

# Сборка
make
```

### Сборка на Windows (Visual Studio)

```bash
mkdir build
cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

## Использование

### Компиляция программы

```bash
./build/my_compiler example.myc -o output.ll
```

### Опции командной строки

- `input_file` — входной файл с исходным кодом
- `-o output_file` — выходной файл для LLVM IR (по умолчанию: `output.ll`)

### Генерация исполняемого файла

После генерации LLVM IR можно скомпилировать его в исполняемый файл:

```bash
# С помощью clang
clang output.ll -o program

# С помощью llc (статическая компиляция)
llc -filetype=obj output.ll -o output.o
clang output.o -o program
```

### Запуск через Makefile

```bash
make run      # Компилирует и запускает компилятор
make test     # Тестирует компилятор
```

## Пример программы

```c
// Вычисление факториала
int factorial(int n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

// Вычисление суммы чисел от 1 до n
int sum(int n) {
    int result = 0;
    int i = 1;
    while (i <= n) {
        result = result + i;
        i = i + 1;
    }
    return result;
}

// Главная функция
void main() {
    int x = 5;
    int fact = factorial(x);
    
    print("Factorial of ");
    print(x);
    print(" is ");
    print(fact);
    print("\n");
    
    int s = sum(10);
    print("Sum from 1 to 10 is ");
    print(s);
    print("\n");
    
    // Пример цикла for
    int i = 0;
    for (int j = 0; j < 5; j = j + 1) {
        i = i + j;
    }
    
    print("Sum of 0+1+2+3+4 is ");
    print(i);
    print("\n");
    
    // Условный оператор
    if (i > 10) {
        print("i is greater than 10\n");
    } else {
        print("i is not greater than 10\n");
    }
}
```

## Планы развития

- [ ] Поддержка массивов
- [ ] Поддержка строк
- [ ] Оптимизации LLVM (O1, O2, O3)
- [ ] Генерация ELF/PE исполняемых файлов напрямую
- [ ] Лучшая обработка ошибок
- [ ] Отладочная информация (DWARF/PDB)
- [ ] Поддержка указателей
- [ ] Улучшенная система типов с поддержкой структур

## Лицензия

MIT License
