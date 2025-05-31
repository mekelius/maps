#include "doctest.h"

#include <sstream>

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/misc_expression.hh"
#include "mapsc/ast/reference.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/parser/layer2.hh"
#include "mapsc/logging_options.hh"

#include "mapsc/ast/value.hh"

#include "mapsc/ast/layer2_expression.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Unary minus by itself should result in a partially applied minus") {
    auto [state, _0, types] = CompilationState::create_test_state();
    auto& ast_store = *state.ast_store_;

    auto value = create_numeric_literal(ast_store, "456", TSL);
    auto minus = create_minus_sign(ast_store, TSL);

    auto expr = create_layer2_expression_testing(ast_store, {minus, value}, TSL);    

    bool success = run_layer2(state, expr);

    CHECK(success);

    CHECK(expr->expression_type == ExpressionType::partially_applied_minus);
    CHECK(std::get<Expression*>(expr->value) == value);
    CHECK(*expr->type == Int);
}

// TEST_CASE("-known_val_ref") {
//     auto [state, _0, types] = CompilationState::create_test_state();
//     auto& ast_store = *state.ast_store_;

//     auto value = create_numeric_literal(ast_store, "456", TSL);
//     auto value_def = ast_store.allocate_definition(DefinitionHeader{"x", TSL}, value);
//     auto value_ref = create_reference(ast_store, value_def, TSL);
//     auto minus = create_minus_sign(ast_store, TSL);

//     auto expr = create_layer2_expression_testing(ast_store, {minus, value_ref}, TSL);    

//     bool success = run_layer2(state, expr);

//     CHECK(success);

//     CHECK(expr->expression_type == ExpressionType::partially_applied_minus);

//     auto inner_value = expr->partially_applied_minus_arg_value();

//     CHECK(inner_value->expression_type == ExpressionType::known_value);
//     CHECK(*inner_value->type == Int);
// }

TEST_CASE("7 + - 2") {
    auto [state, types] = CompilationState::create_test_state_with_builtins();
    auto& ast_store = *state.ast_store_;

    auto plus = create_testing_binary_operator(ast_store, "+", &IntInt_to_Int, 2, 
        Operator::Associativity::left, TSL);

    auto value1 = create_numeric_literal(ast_store, "7", TSL);
    auto plus_ref = create_operator_reference(ast_store, plus, TSL);
    auto minus = create_minus_sign(ast_store, TSL);
    auto value2 = create_numeric_literal(ast_store, "2", TSL);

    auto expr = create_layer2_expression_testing(ast_store, {value1, plus_ref, minus, value2}, TSL);    

    bool success = run_layer2(state, expr);
    CHECK(success);

    CHECK(expr->expression_type == ExpressionType::call);

    auto [callee, args] = expr->call_value();

    CHECK(*callee == *plus);

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

    auto value1 = create_numeric_literal(ast_store, "7", TSL);
    auto minus1 = create_minus_sign(ast_store, TSL);
    auto minus2 = create_minus_sign(ast_store, TSL);
    auto value2 = create_numeric_literal(ast_store, "2", TSL);

    auto expr = create_layer2_expression_testing(ast_store, {value1, minus1, minus2, value2}, TSL);    

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
