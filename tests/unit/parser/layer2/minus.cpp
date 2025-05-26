#include "doctest.h"

#include <sstream>

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/parser/layer2.hh"
#include "mapsc/logging_options.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Unary minus by itself should result in a partially applied minus") {
    auto [state, _0, types] = CompilationState::create_test_state();
    auto& ast_store = *state.ast_store_;

    auto value = Expression::numeric_literal(ast_store, "456", TSL);
    auto minus = Expression::minus_sign(ast_store, TSL);

    auto expr = Expression{ExpressionType::termed_expression, 
            TermedExpressionValue{{minus, value}, db_false}, TSL};    

    run_layer2(state, &expr);

    CHECK(expr.expression_type == ExpressionType::partially_applied_minus);
    CHECK(std::get<Expression*>(expr.value) == value);
    CHECK(*expr.type == Int);
}
