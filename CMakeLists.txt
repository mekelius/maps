cmake_minimum_required(VERSION 4.0.0)

set(CMAKE_C_COMPILER "clang-19")
set(CMAKE_CXX_COMPILER "clang++-19")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


project(Maps)

add_compile_options(-Wall -Wextra)

# ------------------- MAPSC FRONTENT -------------------

add_library(parsing OBJECT
    src/logging.cpp
    src/lang/ast.cpp
    src/lang/types.cpp
    src/parsing/tokens.cpp
    src/parsing/lexer.cpp
    src/parsing/parser_common.cpp
    src/parsing/parser_layer1.cpp
    src/parsing/parser_layer2.cpp
)

# -------------------- VERIFY MAPSC --------------------

add_executable(verify_mapsc
    src/lang/reverse_parse.cpp
    src/verify_main.cpp
)

target_link_libraries(verify_mapsc parsing)

# ----------------------- MAPSC ------------------------

set(CMAKE_LINKER_TYPE "LLD")

add_executable(mapsc 
    src/ir/ir_generator.cpp
    src/ir/ir_output.cpp
    src/main.cpp
)

target_link_libraries(mapsc parsing -lLLVM-19)
target_include_directories(mapsc SYSTEM PUBLIC /usr/lib/llvm-19/include)

target_compile_options(mapsc PRIVATE 
    -fno-exceptions -funwind-tables 
    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
)
