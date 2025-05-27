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

    auto expr = Expression::termed_testing(ast_store, {minus, value}, TSL);    

    bool success = run_layer2(state, expr);

    CHECK(success);

    CHECK(expr->expression_type == ExpressionType::partially_applied_minus);
    CHECK(std::get<Expression*>(expr->value) == value);
    CHECK(*expr->type == Int);
}

TEST_CASE("7 + - 2") {
    auto [state, types] = CompilationState::create_test_state_with_builtins();
    auto& ast_store = *state.ast_store_;

    auto plus = RT_Operator::create_binary("+", External{}, &IntInt_to_Int, 2, 
        Operator::Associativity::left, true, TSL);

    auto value1 = Expression::numeric_literal(ast_store, "7", TSL);
    auto plus_ref = Expression::operator_reference(ast_store, &plus, TSL);
    auto minus = Expression::minus_sign(ast_store, TSL);
    auto value2 = Expression::numeric_literal(ast_store, "2", TSL);

    auto expr = Expression::termed_testing(ast_store, {value1, plus_ref, minus, value2}, TSL);    

    bool success = run_layer2(state, expr);
    CHECK(success);

    CHECK(expr->expression_type == ExpressionType::call);

    auto [callee, args] = expr->call_value();

    CHECK(*callee == plus);

    auto lhs = args.at(0);
    auto rhs = args.at(1);

    CHECK(lhs->expression_type == ExpressionType::known_value);
    CHECK(*lhs->type == Int);
    CHECK(std::get<maps_Int>(lhs->value) == 7);

    CHECK(rhs->expression_type == ExpressionType::call);

    auto [rhs_callee, rhs_args] = rhs->call_value();
    auto rhs_arg = rhs_args.at(0);

    CHECK(rhs_callee == state.special_definitions_.unary_minus);
    CHECK(rhs_arg->expression_type == ExpressionType::known_value);
    CHECK(*rhs_arg->type == Int);

    CHECK(std::get<maps_Int>(rhs_arg->value) == 2);
}

TEST_CASE("7 - - 2") {
    auto [state, types] = CompilationState::create_test_state_with_builtins();
    auto& ast_store = *state.ast_store_;

    auto value1 = Expression::numeric_literal(ast_store, "7", TSL);
    auto minus1 = Expression::minus_sign(ast_store, TSL);
    auto minus2 = Expression::minus_sign(ast_store, TSL);
    auto value2 = Expression::numeric_literal(ast_store, "2", TSL);

    auto expr = Expression::termed_testing(ast_store, {value1, minus1, minus2, value2}, TSL);    

    bool success = run_layer2(state, expr);
    CHECK(success);

    CHECK(expr->expression_type == ExpressionType::call);

    auto [callee, args] = expr->call_value();

    CHECK(callee == state.special_definitions_.binary_minus);

    auto lhs = args.at(0);
    auto rhs = args.at(1);

    CHECK(lhs->expression_type == ExpressionType::known_value);
    CHECK(*lhs->type == Int);
    CHECK(std::get<maps_Int>(lhs->value) == 7);

    CHECK(rhs->expression_type == ExpressionType::call);

    auto [rhs_callee, rhs_args] = rhs->call_value();
    auto rhs_arg = rhs_args.at(0);

    CHECK(rhs_callee == state.special_definitions_.unary_minus);
    CHECK(rhs_arg->expression_type == ExpressionType::known_value);
    CHECK(*rhs_arg->type == Int);

    CHECK(std::get<maps_Int>(rhs_arg->value) == 2);
}
