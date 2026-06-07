cmake_minimum_required(VERSION 3.16)
project(MyCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для Windows
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

# Поиск LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM: ${LLVM_VERSION}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Поиск Flex и Bison
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Генерация парсера
BISON_TARGET(MyParser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
    COMPILE_FLAGS "-d"
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp
)

# Генерация лексера
FLEX_TARGET(MyLexer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scanner.l
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
)

# Зависимости
add_flex_bison_dependency(MyLexer MyParser)

# Исходные файлы (с учётом новой структуры)
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ast/ast.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/codegen/codegen_llvm.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
)

# Заголовочные файлы
set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ast/ast.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/codegen/codegen_llvm.h
    ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp
)

# Создание исполняемого файла
add_executable(my_compiler ${SOURCES} ${HEADERS})

# Включаем директории для заголовков
target_include_directories(my_compiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ast
    ${CMAKE_CURRENT_SOURCE_DIR}/src/codegen
    ${CMAKE_CURRENT_BINARY_DIR}
)

# LLVM библиотеки
llvm_map_components_to_libnames(llvm_libs
    core
    irreader
    support
    native
    passes
    analysis
    target
)

target_link_libraries(my_compiler ${llvm_libs})

# Для Windows добавляем системные библиотеки
if(WIN32)
    target_link_libraries(my_compiler 
        psapi 
        shell32 
        ole32 
        uuid 
        advapi32
    )
endif()