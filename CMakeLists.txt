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
    libmaps
)

# --------------------- LIBMAPS -------------------

add_library(libmaps SHARED
    libmaps/maps_datatypes.h
    libmaps/libmaps.h
    libmaps/libmaps.c
)

set_property(TARGET libmaps PROPERTY OUTPUT_NAME maps)

# -------------------- MAPSC MAPSC_COMMON --------------------

add_library(mapsc_common OBJECT
    src/mapsc/source_location.cpp
    src/mapsc/logging.cpp
)

add_executable(mapsc_common_unit_tests
    tests/unit/tests_main.cpp
    tests/unit/logging.cpp
)

set_property(TARGET mapsc_common_unit_tests 
    PROPERTY EXCLUDE_FROM_ALL True)
target_include_directories(mapsc_common_unit_tests SYSTEM PRIVATE doctest tests)
target_link_libraries(mapsc_common_unit_tests
    libmaps
    mapsc_common
)
add_test(NAME mapsc_common_unit_tests COMMAND mapsc_common_unit_tests)

# -------------------- MAPSC TYPES AND AST --------------------

add_library(types_and_ast OBJECT
    src/mapsc/types/type.cpp
    src/mapsc/types/casts.cpp
    src/mapsc/types/function_type.cpp
    src/mapsc/types/type_constructor.cpp
    src/mapsc/types/type_store.cpp

    src/mapsc/pragma.cpp

    src/mapsc/ast/expression.cpp
    src/mapsc/ast/expression_properties.cpp
    src/mapsc/ast/statement.cpp
    src/mapsc/ast/definition.cpp
    src/mapsc/ast/definition_body.cpp
    src/mapsc/ast/let_definition.cpp
    src/mapsc/ast/function_definition.cpp
    src/mapsc/ast/operator.cpp
    src/mapsc/ast/external.cpp
    src/mapsc/ast/ast_store.cpp
    src/mapsc/ast/identifier.cpp
    src/mapsc/ast/reference.cpp
    src/mapsc/ast/scope.cpp
    src/mapsc/ast/value.cpp
    src/mapsc/ast/layer2_expression.cpp
    src/mapsc/ast/misc_expression.cpp
    src/mapsc/ast/call_expression.cpp
    src/mapsc/ast/lambda.cpp

    src/mapsc/ast/expressions/casts.cpp
    src/mapsc/ast/expressions/ternary.cpp

    src/mapsc/ast/test_helpers/test_definition.cpp

    src/mapsc/compilation_state.cpp # what to do about this?
    src/mapsc/procedures/create_call.cpp # should be moved into ast
)

add_executable(types_and_ast_unit_tests
    tests/unit/tests_main.cpp

    tests/unit/builtin.cpp
    tests/unit/ast/builtin.cpp

    tests/unit/pragmas.cpp

    tests/unit/ast/ast_store.cpp
    tests/unit/ast/scope.cpp
    tests/unit/ast/definition.cpp
    tests/unit/ast/value_and_reference.cpp

    tests/unit/types/types.cpp
    tests/unit/types/type_casts.cpp
    tests/unit/types/type_store.cpp
    tests/unit/types/function_type.cpp    
)

set_property(TARGET types_and_ast_unit_tests 
    PROPERTY EXCLUDE_FROM_ALL True)
target_include_directories(types_and_ast_unit_tests SYSTEM PRIVATE doctest tests)
target_link_libraries(types_and_ast_unit_tests 
    libmaps
    mapsc_common
    types_and_ast 
)
add_test(NAME types_and_ast_unit_tests COMMAND types_and_ast_unit_tests)

# -------------------- MAPSC PROCEDURES --------------------

add_library(procedures OBJECT
    src/mapsc/procedures/inline.cpp
    src/mapsc/procedures/coerce_type.cpp
    src/mapsc/procedures/simplify.cpp
    src/mapsc/procedures/concretize.cpp
    src/mapsc/procedures/type_check.cpp
    src/mapsc/procedures/name_resolution.cpp
    src/mapsc/procedures/reverse_parse.cpp
    src/mapsc/procedures/cleanup.cpp
)

add_executable(procedures_unit_tests
    tests/unit/tests_main.cpp

    tests/unit/procedures/name_resolution.cpp
    tests/unit/procedures/concretize.cpp
    tests/unit/procedures/create_call.cpp
    tests/unit/procedures/inline.cpp
)

set_property(TARGET procedures_unit_tests 
    PROPERTY EXCLUDE_FROM_ALL True)
target_include_directories(procedures_unit_tests SYSTEM PRIVATE doctest tests)
target_link_libraries(procedures_unit_tests 
    mapsc_common
    types_and_ast
    procedures
    libmaps
)
add_test(NAME procedures_unit_tests COMMAND procedures_unit_tests)

# -------------------- MAPSC PARSER LAYER1 --------------------

add_library(parser_layer1 OBJECT
    src/mapsc/parser/token.cpp
    src/mapsc/parser/lexer.cpp
    src/mapsc/parser/layer1.cpp

    src/mapsc/parser/layer1/layer1.cpp
    src/mapsc/parser/layer1/conditional.cpp
    src/mapsc/parser/layer1/loop.cpp
    src/mapsc/parser/layer1/definition.cpp
    src/mapsc/parser/layer1/expression.cpp
    src/mapsc/parser/layer1/pragma.cpp
    src/mapsc/parser/layer1/statement.cpp
    src/mapsc/parser/layer1/layer2_expression.cpp
    src/mapsc/parser/layer1/terminal.cpp
)

add_executable(parser_layer1_unit_tests
    tests/unit/tests_main.cpp

    tests/unit/parser/layer1/lexer.cpp
    tests/unit/parser/layer1/basics.cpp
    tests/unit/parser/layer1/block.cpp
    tests/unit/parser/layer1/definition.cpp
    tests/unit/parser/layer1/conditional.cpp
    tests/unit/parser/layer1/loop.cpp
    tests/unit/parser/layer1/lambda.cpp
    tests/unit/parser/layer1/minus.cpp
    tests/unit/parser/layer1/termed.cpp
    tests/unit/parser/layer1/ternary.cpp
)

set_property(TARGET parser_layer1_unit_tests 
    PROPERTY EXCLUDE_FROM_ALL True)

target_link_libraries(parser_layer1_unit_tests 
    libmaps
    mapsc_common
    parser_layer1 
    types_and_ast
    procedures
)
target_include_directories(parser_layer1_unit_tests SYSTEM PRIVATE doctest tests)
add_test(NAME parser_layer1_unit_tests COMMAND parser_layer1_unit_tests)

# -------------------- MAPSC PARSER LAYER2 --------------------

add_library(parser_layer2 OBJECT
    src/mapsc/parser/layer2.cpp
    src/mapsc/parser/layer2/layer2.cpp
)

add_executable(parser_layer2_unit_tests
    tests/unit/tests_main.cpp

    tests/unit/parser/layer2/unary_minus.cpp
    tests/unit/parser/layer2/unary_operators.cpp
    tests/unit/parser/layer2/basics.cpp
    tests/unit/parser/layer2/calls.cpp
    tests/unit/parser/layer2/minus.cpp
    tests/unit/parser/layer2/partial_application.cpp
    tests/unit/parser/layer2/precedence.cpp
    tests/unit/parser/layer2/type_declarations.cpp
    tests/unit/parser/layer2/type_specifiers.cpp
)

set_property(TARGET parser_layer2_unit_tests 
    PROPERTY EXCLUDE_FROM_ALL True)

target_link_libraries(parser_layer2_unit_tests
    libmaps
    mapsc_common
    parser_layer2
    types_and_ast
    procedures
)
target_include_directories(parser_layer2_unit_tests SYSTEM PRIVATE doctest tests)
add_test(NAME parser_layer2_unit_tests COMMAND parser_layer2_unit_tests)

# -------------------- MAPSC TRANSFORM STAGE --------------------

add_library(transforms OBJECT
    src/mapsc/transform_stage.cpp
)

target_link_libraries(transforms libmaps types_and_ast)

add_executable(transforms_unit_tests 
    tests/unit/tests_main.cpp
)

set_property(TARGET transforms_unit_tests 
    PROPERTY EXCLUDE_FROM_ALL True)

target_link_libraries(transforms_unit_tests 
    libmaps
    mapsc_common
    types_and_ast
    procedures
    transforms
)
target_include_directories(transforms_unit_tests SYSTEM PRIVATE doctest tests)
add_test(NAME transforms_unit_tests COMMAND transforms_unit_tests)

# ------------------------ IR_GEN -------------------------

add_library(ir_gen OBJECT
    src/mapsc/llvm_ir_gen/ir_generator.cpp
    src/mapsc/llvm_ir_gen/ir_builtins.cpp
    src/mapsc/llvm_ir_gen/type_mapping.cpp
    src/mapsc/llvm_ir_gen/function_store.cpp
)

target_include_directories(ir_gen SYSTEM PUBLIC /usr/lib/llvm-19/include)

add_executable(ir_gen_unit_tests
    tests/unit/tests_main.cpp
)
set_property(TARGET ir_gen_unit_tests 
    PROPERTY EXCLUDE_FROM_ALL True)

target_link_libraries(ir_gen_unit_tests 
    libmaps
    mapsc_common
    types_and_ast
    parser_layer1
    parser_layer2
    procedures
    transforms
    ir_gen
    -lLLVM-19
)
target_include_directories(ir_gen_unit_tests SYSTEM PRIVATE doctest tests)
add_test(NAME ir_gen_unit_tests COMMAND ir_gen_unit_tests)

# not written yet, mostly here to check that stuff compiles
# ------------------------- LIBMAPSC -------------------------

add_library(libmapsc INTERFACE)

target_link_libraries(libmapsc INTERFACE
    libmaps
    $<TARGET_OBJECTS:mapsc_common>
    $<TARGET_OBJECTS:types_and_ast>
    $<TARGET_OBJECTS:parser_layer1>
    $<TARGET_OBJECTS:parser_layer2>
    $<TARGET_OBJECTS:procedures>
    $<TARGET_OBJECTS:transforms>
    $<TARGET_OBJECTS:ir_gen>
    -lLLVM-19
)

set_target_properties(libmapsc PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES CXX)

add_executable(integration_tests
    tests/unit/tests_main.cpp
)
set_property(TARGET integration_tests 
    PROPERTY EXCLUDE_FROM_ALL True)

target_link_libraries(integration_tests 
    libmaps
    libmapsc
)
target_include_directories(integration_tests SYSTEM PRIVATE doctest tests)
add_test(NAME integration_tests COMMAND integration_tests)

# ------------------------- MAPSC -------------------------

# add_executable(mapsc
#     src/mapsc/llvm/obj_output.cpp
#     src/main.cpp
# )

# target_link_libraries(mapsc ir_gen libmaps -lLLVM-19)
# target_include_directories(mapsc SYSTEM PUBLIC /usr/lib/llvm-19/include)

# target_compile_options(mapsc PRIVATE 
#     -fno-exceptions -funwind-tables 
#     -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
# )

# ------------------------- MAPSCI ------------------------

add_executable(mapsci
    src/mapsci/init_llvm.cpp
    src/mapsci/jit_manager.cpp
    src/mapsci/cl_options.cpp

    src/mapsci/repl.cpp
    src/mapsci/repl/repl.cpp
    src/mapsci/repl/commands.cpp
    src/mapsci/repl/compilation_pipeline.cpp
    
    src/mapsci_main.cpp
)

target_link_libraries(mapsci 
    -lLLVM-19 
    libmapsc
    libmaps
    
    -ledit 
    -lncurses
)
target_include_directories(mapsci SYSTEM PUBLIC 
    /usr/lib/llvm-19/include
    /usr/include/editline
)

target_compile_options(mapsci PRIVATE 
    -fno-exceptions -funwind-tables
    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
)

# ---------------------- VERIFY MAPSC ----------------------

# add_executable(mapsc-verify
#     src/verify_main.cpp
# )

# target_link_libraries(mapsc-verify)

# ------------------------ TESTS ------------------------

add_custom_target(unit_tests)
add_dependencies(unit_tests 
    mapsc_common_unit_tests 
    types_and_ast_unit_tests
    parser_layer1_unit_tests
    parser_layer2_unit_tests
    transforms_unit_tests
    procedures_unit_tests
    ir_gen_unit_tests
    integration_tests
)

configure_file(tests/e2e/lit.site.cfg.py.in lit.site.cfg.py @ONLY)
add_test(NAME e2e_tests COMMAND lit "${CMAKE_CURRENT_BINARY_DIR}" -v)

add_custom_target(all_tests)
add_dependencies(all_tests mapsci unit_tests)

# ------------------------- DSIR -------------------------

# add_library(dsir OBJECT
#     src/mapsc/dsir/parse_dsir.cpp
# )

# target_link_libraries(dsir)

# --------------------- DSIR UNIT TESTS -----------------------

# add_executable(dsir_unit_tests
#     tests/unit/tests_main.cpp

#     tests/unit/dsir/basics.cpp
# )

# set_property(TARGET dsir_unit_tests 
#     PROPERTY EXCLUDE_FROM_ALL True)

# target_link_libraries(dsir_unit_tests dsir)
# target_include_directories(dsir_unit_tests SYSTEM PRIVATE doctest tests)
