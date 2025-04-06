cmake_minimum_required(VERSION 4.0.0)

set(CMAKE_C_COMPILER "clang-19")
set(CMAKE_CXX_COMPILER "clang++-19")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


project(Maps)

add_compile_options(-Wall -Wpedantic)

# ------------------- MAPSC FRONTENT -------------------

add_library(mapsc_frontend OBJECT
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp    
)

# -------------------- VERIFY MAPSC --------------------

add_executable(verify_mapsc
    src/verify_main.cpp
)
target_link_libraries(verify_mapsc mapsc_frontend)

# ----------------------- MAPSC ------------------------

set(CMAKE_LINKER_TYPE "LLD")

add_executable(mapsc 
    src/ir_generator.cpp
    src/output.cpp
    src/main.cpp
)

target_link_libraries(mapsc mapsc_frontend -lLLVM-19)

target_compile_options(mapsc PRIVATE
    -I/usr/lib/llvm-19/include 
    -fno-exceptions -funwind-tables 
    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
)
