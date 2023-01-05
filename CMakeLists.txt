cmake_minimum_required(VERSION 3.20)
project(KubeFormat)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(LLVM_LINK_COMPONENTS support)

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

# set(CMAKE_CXX_FLAGS "-Wall -g3 -O0 -fno-rtti ${LLVM_COMPILE_FLAGS}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

add_executable(KubeFormat
    ClangTooling.hpp
    KubeFormat.cpp
    Format.cpp
    Format.hpp
)

target_precompile_headers(KubeFormat
PRIVATE
    ClangTooling.hpp
)

target_link_libraries(KubeFormat
PRIVATE
    ${CLANG_LIBS}
    ${LLVM_LIBS_CORE}
)
target_link_options(KubeFormat
PRIVATE
    ${LLVM_LDFLAGS}
)

target_compile_options(KubeFormat
PRIVATE
    ${LLVM_COMPILE_FLAGS}
)