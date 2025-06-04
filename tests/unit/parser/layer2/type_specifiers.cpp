#include "doctest.h"

#include <string>
#include <sstream>

#include "mapsc/types/type.hh"
#include "mapsc/ast/value.hh"

#include "mapsc/ast/layer2_expression.hh"
#include "mapsc/ast/reference.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/parser/layer2.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;

TEST_CASE("Should parse a numberliteral with the correct type") {
    auto [state, types] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;

    auto expr = create_layer2_expression_testing(ast, {
        create_type_reference(ast, &Int, TSL),
        create_numeric_literal(ast, "23", TSL)
    }, TSL);

    run_layer2(state, expr);

    CHECK(*expr->type == Int);
}

TEST_CASE("Should be able to cast a string literal into Int") {
    auto [state, types] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;
    
    auto expr = create_layer2_expression_testing(ast, {
        create_type_reference(ast, &Int, TSL),
        create_string_literal(ast, "23", TSL)
    }, TSL);

    run_layer2(state, expr);

    CHECK(*expr->type == Int);
    CHECK(std::get<maps_Int>(expr->value) == 23);
}
