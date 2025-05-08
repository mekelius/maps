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

include_directories(
    src 
    corelib
)

# ------------------- MAPSC CORELIB ------------------

add_library(corelib SHARED
    corelib/maps_datatypes.h
    corelib/corelib.h
    corelib/corelib.c
)

set_property(TARGET corelib PROPERTY OUTPUT_NAME maps_core)

# ------------------- MAPSC PARSER -------------------

add_library(mapsc_frontend OBJECT
    src/mapsc/logging.cpp

    src/mapsc/pragma.cpp
    src/mapsc/builtins.cpp
    src/mapsc/name_resolution.cpp
    src/mapsc/inline.cpp
    src/mapsc/simplify.cpp

    src/mapsc/types/type.cpp
    src/mapsc/types/type_defs.cpp
    src/mapsc/types/casts.cpp
    src/mapsc/types/type_checking.cpp
    src/mapsc/types/function_type.cpp
    src/mapsc/types/type_store.cpp
    
    src/mapsc/ast/ast_node.cpp
    src/mapsc/ast/callable.cpp
    src/mapsc/ast/ast_store.cpp
    src/mapsc/ast/scope.cpp
    
    src/mapsc/parser/token.cpp
    src/mapsc/parser/lexer.cpp
    src/mapsc/parser/parser_layer1.cpp
    src/mapsc/parser/parser_layer2.cpp
    src/mapsc/parser/reverse_parse.cpp
    src/mapsc/parser/full_parse.cpp
)

target_link_libraries(mapsc_frontend corelib)

# ---------------------- IR_GEN ------------------------

add_library(ir_gen OBJECT
    src/mapsc/llvm/ir_generator.cpp
    src/mapsc/llvm/ir_builtins.cpp
    src/mapsc/llvm/type_mapping.cpp
    src/mapsc/llvm/function_store.cpp
)

target_link_libraries(ir_gen parser -lLLVM-19)
target_include_directories(ir_gen SYSTEM PUBLIC /usr/lib/llvm-19/include)

# ----------------------- MAPSC ------------------------

add_executable(mapsc
    src/mapsc_main.cpp
    src/mapsc/llvm/obj_output.cpp
)

target_link_libraries(mapsc mapsc_frontend ir_gen corelib -lLLVM-19)
target_include_directories(mapsc SYSTEM PUBLIC /usr/lib/llvm-19/include)

target_compile_options(mapsc PRIVATE 
    -fno-exceptions -funwind-tables 
    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
)

# ----------------------- MAPSCi -----------------------

add_executable(mapsci
    src/mapsci_main.cpp
    src/mapsci/repl.cpp
)

target_link_libraries(mapsci mapsc_frontend ir_gen corelib -lLLVM-19 -ledit -lncurses)
target_include_directories(mapsci SYSTEM PUBLIC 
    /usr/lib/llvm-19/include
    /usr/include/editline
)

target_compile_options(mapsci PRIVATE 
    -fno-exceptions -funwind-tables
    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
)

# -------------------- VERIFY MAPSC --------------------

add_executable(verify_mapsc
    src/verify_mapsc_main.cpp
)

target_link_libraries(verify_mapsc mapsc_frontend)

# --------------------- UNIT_TESTS ---------------------

add_executable(tests
    tests/tests_main.cpp
    tests/ast.cpp
    tests/pragmas.cpp
    tests/layer1.cpp
    tests/layer2.cpp
    tests/type_specifiers.cpp
    tests/type_casts.cpp
    tests/function_casts.cpp
    tests/inline.cpp
)

target_link_libraries(tests mapsc_frontend)

target_include_directories(tests SYSTEM PRIVATE doctest tests)
