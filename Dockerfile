FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ gcc cmake flex bison \
    llvm-14 llvm-14-dev libllvm14 \
    libffi-dev libxml2-dev zlib1g-dev libtinfo-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копируем исходники
COPY src/ ./src/
COPY CMakeLists.txt .

# Исправляем CMakeLists.txt
RUN sed -i 's/project(MyCompiler LANGUAGES CXX)/project(MyCompiler LANGUAGES C CXX)/' CMakeLists.txt

# Исправляем scanner.l: добавляем #include <unistd.h> и убираем nounistd
RUN sed -i '1a#include <unistd.h>' src/scanner.l && \
    sed -i 's/%option nounistd//' src/scanner.l

RUN mkdir build && cd build && \
    cmake .. -DCMAKE_CXX_STANDARD=17 \
             -DCMAKE_C_STANDARD=11 \
             -DLLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm && \
    cmake --build . --config Release

ENTRYPOINT ["./build/my_compiler"]