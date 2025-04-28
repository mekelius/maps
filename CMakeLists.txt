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
    -Wno-error=unused-function
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

include_directories(src)

# ------------------- MAPSC BUILTINS ------------------

add_library(maps_builtins SHARED
    maps_builtins/maps_builtins.h
    maps_builtins/maps_builtins.c
)

# ------------------- MAPSC PARSER -------------------

add_library(parser OBJECT
    src/logging.cpp

    src/lang/type.cpp
    src/lang/type_defs.cpp
    src/lang/pragma.cpp
    src/lang/builtins.cpp
    src/lang/casts.cpp
    
    src/ast/ast_node.cpp
    src/ast/callable.cpp
    src/ast/ast.cpp
    src/ast/scope.cpp
    
    src/parser/token.cpp
    src/parser/lexer.cpp
    src/parser/parser_layer1.cpp
    src/parser/parser_layer2.cpp
    src/parser/name_resolution.cpp
    src/parser/type_inference.cpp
    src/parser/type_declaration.cpp
    src/parser/full_parse.cpp
    src/parser/reverse_parse.cpp
)

# ---------------------- IR_GEN ------------------------

add_library(ir_gen OBJECT
    src/ir/ir_generator.cpp
    src/ir/ir_builtins.cpp
    src/ir/type_mapping.cpp
)

target_link_libraries(ir_gen parser -lLLVM-19)
target_include_directories(ir_gen SYSTEM PUBLIC /usr/lib/llvm-19/include)

# ----------------------- MAPSC ------------------------

add_executable(mapsc
    src/mapsc_main.cpp
    src/ir/obj_output.cpp
)

target_link_libraries(mapsc parser ir_gen maps_builtins -lLLVM-19)
target_include_directories(mapsc SYSTEM PUBLIC /usr/lib/llvm-19/include)

target_compile_options(mapsc PRIVATE 
    -fno-exceptions -funwind-tables 
    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
)

# ----------------------- MAPSCi -----------------------

add_executable(mapsci
    src/mapsci_main.cpp
    src/repl.cpp
)

target_link_libraries(mapsci parser ir_gen maps_builtins -lLLVM-19)
target_include_directories(mapsci SYSTEM PUBLIC /usr/lib/llvm-19/include)

target_compile_options(mapsci PRIVATE 
    -fno-exceptions -funwind-tables 
    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
)

# -------------------- VERIFY MAPSC --------------------

add_executable(verify_mapsc
    src/verify_mapsc_main.cpp
)

target_link_libraries(verify_mapsc parser)

# ----------------- PARSer UNIT_TESTS -----------------

add_executable(parser_unit_tests
    tests/tests_main.cpp
    tests/layer2_tests.cpp
    tests/ast_tests.cpp
    tests/type_specifiers.cpp
)

target_link_libraries(parser_unit_tests parser)
target_include_directories(parser_unit_tests SYSTEM PRIVATE doctest)
