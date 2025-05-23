#include "doctest.h"

#include <string>
#include <sstream>

#include "mapsc/types/type.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/parser/layer2.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;

TEST_CASE("Should parse a numberliteral with the correct type") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;

    auto expr = Expression::termed(ast, {
        Expression::type_reference(ast, &Int, {0,0}),
        Expression::numeric_literal(ast, "23", {0,0})
    }, {0,0});

    run_layer2(state, expr);

    CHECK(*expr->type == Int);
}

TEST_CASE("Should be able to cast a string literal into Int") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;
    
    auto expr = Expression::termed(ast, {
        Expression::type_reference(ast, &Int, {0,0}),
        Expression::string_literal(ast, "23", {0,0})
    }, {0,0});

    run_layer2(state, expr);

    CHECK(*expr->type == Int);
    CHECK(std::get<maps_Int>(expr->value) == 23);
}
