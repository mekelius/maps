#include "doctest.h"

#include "mapsc/ast/expression.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/parser/parser_layer2.hh"

using namespace std;
using namespace Maps;

TEST_CASE("prefix purely unary operator") {
    auto [state, _0, types] = CompilationState::create_test_state();

    auto op = Operator{"!", External{}, *types->get_function_type(Boolean, {&Boolean}, true), 
        {UnaryFixity::prefix}, TSL};
    auto op_ref = Expression{ExpressionType::operator_reference, TSL, &op};
    auto value = Expression{ExpressionType::value, TSL, true, &Boolean};
    auto expr = Expression{ExpressionType::termed_expression, TSL, 
        TermedExpressionValue{{&op_ref, &value}}};

    TermedExpressionParser{&state, &expr}.run();

    CHECK(state.is_valid);

    CHECK(expr.expression_type == ExpressionType::call);
    
    auto [callee, args] = expr.call_value();

    CHECK(args.size() == 1);
    CHECK(**args.begin() == value);
    CHECK(callee == &op);
}

TEST_CASE("postfix purely unary operator") {
    auto [state, _0, types] = CompilationState::create_test_state();

    auto op = Operator{"!", External{}, *types->get_function_type(Boolean, {&Boolean}, true), 
        {UnaryFixity::postfix}, TSL};
    auto op_ref = Expression{ExpressionType::operator_reference, TSL, &op, op.get_type()};
    auto value = Expression{ExpressionType::value, TSL, true, &Boolean};
    auto expr = Expression{ExpressionType::termed_expression, TSL, 
        TermedExpressionValue{{&value, &op_ref}}};

    TermedExpressionParser{&state, &expr}.run();

    CHECK(state.is_valid);

    CHECK(expr.expression_type == ExpressionType::call);
    
    auto [callee, args] = expr.call_value();

    CHECK(args.size() == 1);
    CHECK(**args.begin() == value);
    CHECK(callee == &op);
}