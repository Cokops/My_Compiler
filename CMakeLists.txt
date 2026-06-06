cmake_minimum_required(VERSION 3.16)
project(MyCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск Flex и Bison
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Поиск LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM: ${LLVM_VERSION}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM libraries: ${LLVM_LIBRARY_DIRS}")

# Добавляем флаги компиляции LLVM
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Генерация файлов Flex и Bison
bison_target(parser
    src/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp
    COMPILE_FLAGS "-d"
)

flex_target(scanner
    src/scanner.l
    ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp
)

# Создание статической библиотеки из Flex/Bison кода
add_flex_bison_dependency(scanner parser)

# Исходные файлы проекта
set(SOURCES
    src/main.cpp
    src/ast.cpp
    src/semantic_analyzer.cpp
    src/codegen_llvm.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
    ${FLEX_SCANNER_FILE}
)

# Создание исполняемого файла
add_executable(my_compiler ${SOURCES})
target_link_libraries(my_compiler
    ${LLVM_LIBRARIES}
    ${FLEX_LIBRARIES}
    ${BISON_LIBRARIES}
)

# Настройка путей к библиотекам LLVM
if(LLVM_LINK_MODE STREQUAL "STATIC")
    # Для статической линковки добавляем зависимости
    llvm_map_components_to_libnames(LLVM_LIBS
        core
        irreader
        linker
        native
        nativecodegen
        support
    )
    target_link_libraries(my_compiler ${LLVM_LIBS})
endif()
