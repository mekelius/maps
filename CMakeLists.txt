cmake_minimum_required(VERSION 4.0.0)

set(CMAKE_C_COMPILER "clang-19")
set(CMAKE_CXX_COMPILER "clang++-19")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Maps)

add_compile_options(-Wall -Wextra -Werror 
    -Wno-error=unused-parameter 
    -Wno-error=unused-but-set-variable 
    -Wno-error=unused-variable
    -Wno-error=unused-private-field
)

# ------------------- MAPSC BUILTINS ------------------

add_library(maps_builtins SHARED
    maps_builtins/maps_builtins.h
    maps_builtins/maps_builtins.c
)


# ------------------- MAPSC PARSING -------------------

add_library(parsing OBJECT
    src/logging.cpp
    src/lang/ast.cpp
    src/lang/types.cpp
    src/lang/pragmas.cpp
    src/lang/builtins.cpp
    src/parsing/tokens.cpp
    src/parsing/lexer.cpp
    src/parsing/parser_layer1.cpp
    src/parsing/parser_layer2.cpp
    src/parsing/name_resolution.cpp
    src/parsing/full_parse.cpp
)


# ---------------------- IR_GEN ------------------------

add_library(ir_gen OBJECT
    src/ir/ir_generator.cpp
    src/ir/ir_builtins.cpp
    src/ir/ir_types.cpp
)

target_link_libraries(ir_gen parsing -lLLVM-19)
target_include_directories(ir_gen SYSTEM PUBLIC /usr/lib/llvm-19/include)


# ----------------------- MAPSC ------------------------

add_executable(mapsc
    src/mapsc_main.cpp
    src/ir/obj_output.cpp
)

target_link_libraries(mapsc parsing ir_gen maps_builtins -lLLVM-19)
target_include_directories(mapsc SYSTEM PUBLIC /usr/lib/llvm-19/include)

target_compile_options(mapsc PRIVATE 
    -fno-exceptions -funwind-tables 
    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
)


# ----------------------- MAPSCi -----------------------

add_executable(mapsci
    src/mapsci_main.cpp
    src/repl.cpp
    src/lang/reverse_parse.cpp
)

target_link_libraries(mapsci parsing ir_gen maps_builtins -lLLVM-19)
target_include_directories(mapsci SYSTEM PUBLIC /usr/lib/llvm-19/include)

target_compile_options(mapsci PRIVATE 
    -fno-exceptions -funwind-tables 
    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
)


# -------------------- VERIFY MAPSC --------------------

add_executable(verify_mapsc
    src/lang/reverse_parse.cpp
    src/verify_mapsc_main.cpp
)

target_link_libraries(verify_mapsc parsing)


# ----------------- PARSING UNIT_TESTS -----------------

add_executable(parsing_unit_tests
    tests/tests_main.cpp
    tests/layer2_tests.cpp
    tests/types_tests.cpp
    tests/ast_tests.cpp
)

target_link_libraries(parsing_unit_tests parsing)
target_include_directories(parsing_unit_tests SYSTEM PRIVATE doctest)
