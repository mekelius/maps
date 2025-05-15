cmake_minimum_required(VERSION 4.0.0)

set(CMAKE_C_COMPILER "clang-19")
set(CMAKE_CXX_COMPILER "clang++-19")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Maps)

include(CTest)
set(CMAKE_CTEST_ARGUMENTS "--output-on-failure")

add_compile_options(-Wall -Wextra -Werror 
    -Wno-error=unused-parameter 
    -Wno-error=unused-but-set-variable 
    -Wno-error=unused-variable
    -Wno-error=unused-private-field
    -Wno-error=unused-function
    -fdiagnostics-color=always # needed to get colored output with ninja
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

include_directories(
    src 
    corelib
)

# --------------------- MAPSC CORELIB -------------------

add_library(corelib SHARED
    corelib/maps_datatypes.h
    corelib/corelib.h
    corelib/corelib.c
)

set_property(TARGET corelib PROPERTY OUTPUT_NAME maps_core)

# -------------------- MAPSC FRONTEND --------------------

add_library(mapsc_frontend OBJECT
    src/mapsc/logging.cpp
    src/mapsc/compiler_options.cpp
    src/mapsc/pragma.cpp
    src/mapsc/compilation_state.cpp

    src/mapsc/types/type.cpp
    src/mapsc/types/casts.cpp
    src/mapsc/types/function_type.cpp
    src/mapsc/types/type_store.cpp

    src/mapsc/ast/expression.cpp
    src/mapsc/ast/statement.cpp
    src/mapsc/ast/callable.cpp
    src/mapsc/ast/ast_store.cpp

    src/mapsc/builtins.cpp

    src/mapsc/procedures/inline.cpp
    src/mapsc/procedures/coerce_type.cpp
    src/mapsc/procedures/simplify.cpp
    src/mapsc/procedures/concretize.cpp
    src/mapsc/procedures/type_check.cpp
    src/mapsc/procedures/name_resolution.cpp
    src/mapsc/procedures/reverse_parse.cpp

    src/mapsc/parser/token.cpp
    src/mapsc/parser/lexer.cpp
    src/mapsc/parser/parser_layer1.cpp
    src/mapsc/parser/parser_layer2.cpp

    src/mapsc/process_source.cpp
)

target_link_libraries(mapsc_frontend corelib)

# ------------------------ IR_GEN -------------------------

add_library(ir_gen OBJECT
    src/mapsc/llvm/ir_generator.cpp
    src/mapsc/llvm/ir_builtins.cpp
    src/mapsc/llvm/type_mapping.cpp
    src/mapsc/llvm/function_store.cpp
)

target_link_libraries(ir_gen mapsc_frontend -lLLVM-19)
target_include_directories(ir_gen SYSTEM PUBLIC /usr/lib/llvm-19/include)

# ------------------------- MAPSC -------------------------

add_executable(mapsc
    src/mapsc/llvm/obj_output.cpp
    src/mapsc_main.cpp
)

target_link_libraries(mapsc mapsc_frontend ir_gen corelib -lLLVM-19)
target_include_directories(mapsc SYSTEM PUBLIC /usr/lib/llvm-19/include)

target_compile_options(mapsc PRIVATE 
    -fno-exceptions -funwind-tables 
    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
)

# ------------------------- MAPSCi ------------------------

add_executable(mapsci
    src/mapsci/repl.cpp
    src/mapsci_main.cpp
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

# ---------------------- VERIFY MAPSC ----------------------

add_executable(mapsc-verify
    src/mapsc_verify_main.cpp
)

target_link_libraries(mapsc-verify mapsc_frontend)

# ----------------------- UNIT TESTS -----------------------

add_executable(unit_tests
    tests/tests_main.cpp
    
    tests/full_runs.cpp

    tests/pragmas.cpp
    tests/compiler_options.cpp

    tests/ast/ast.cpp

    tests/parser/layer1.cpp
    tests/parser/layer2.cpp
    
    tests/types/type_casts.cpp
    tests/types/type_store.cpp
    tests/types/type_specifiers.cpp
    tests/types/function_type.cpp
    
    tests/procedures/inline.cpp
    tests/procedures/concretize.cpp
)

set_property(TARGET unit_tests 
    PROPERTY EXCLUDE_FROM_ALL True)

target_link_libraries(unit_tests mapsc_frontend)
target_include_directories(unit_tests SYSTEM PRIVATE doctest tests)

# ------------------------ E2E TESTS ------------------------

configure_file(tests/e2e/lit.site.cfg.py.in lit.site.cfg.py @ONLY)
configure_file(tests/e2e/run_lit.py test/run_lit.py @ONLY)


add_test(NAME unit_tests COMMAND unit_tests)
add_test(NAME e2e_tests COMMAND ./test/run_lit.py "${CMAKE_CURRENT_BINARY_DIR}" -v)