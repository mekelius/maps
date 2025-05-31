#include "doctest.h"

#include <sstream>

#include "mapsc/compilation_state.hh"
#include "mapsc/logging_options.hh"

#include "mapsc/types/type_defs.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/ast/misc_expression.hh"
#include "mapsc/ast/call_expression.hh"
#include "mapsc/ast/layer2_expression.hh"
#include "mapsc/ast/reference.hh"

#include "mapsc/parser/layer2.hh"

using namespace Maps;
using namespace std;


TEST_CASE("Layer2 should handle type specifiers") {
    auto [state, _0, types] = CompilationState::create_test_state();

    auto type_specifier = Expression{ExpressionType::type_reference, &Int, TSL};
    auto value = create_numeric_literal(*state.ast_store_, "32", TSL);

    SUBCASE("Int \"32\"") {
        auto expr = create_layer2_expression_testing(*state.ast_store_, {&type_specifier, value}, TSL);

        run_layer2(state, expr);

        CHECK(*expr->type == Int);
        CHECK(expr->expression_type == ExpressionType::known_value);

        CHECK(*value->type == Int);
        CHECK(holds_alternative<maps_Int>(value->value));
        CHECK(get<maps_Int>(value->value) == 32);
    }

    SUBCASE("Int \"32\" + 987") {
        auto op = create_testing_binary_operator(*state.ast_store_, "+", 
            types->get_function_type(&Int, {&Int, &Int}, true), 500, TSL);
        
        auto op_ref = create_operator_reference(*state.ast_store_, op, TSL);
        auto rhs = create_numeric_literal(*state.ast_store_, "987", TSL);
        auto expr = create_layer2_expression_testing(*state.ast_store_, 
            {&type_specifier, value, op_ref, rhs}, TSL);

        run_layer2(state, expr);

        CHECK(*expr->type == Int);
        CHECK(expr->expression_type == ExpressionType::call);

        auto [callee, args] = expr->call_value();

        CHECK(args.size() == 2);
        CHECK(*callee == *op);
        CHECK(*value->type == Int);
        CHECK(holds_alternative<maps_Int>(value->value));
        CHECK(get<maps_Int>(value->value) == 32);
        CHECK(*args.at(0) == *value);
    }
}

TEST_CASE("Should apply a type declaration") {
    auto [state, _0, types] = CompilationState::create_test_state();
    auto& ast_store = *state.ast_store_;

    auto value = create_numeric_literal(ast_store, "23", TSL);
    auto type = create_type_reference(ast_store, &Float, TSL);

    auto outer = create_layer2_expression_testing(ast_store, {type, value}, TSL);

    auto success = run_layer2(state, outer);

    CHECK(success);
    CHECK(*outer->type == Float);
}

TEST_CASE("Partially applied minus and type declaration") {
    auto [state, types] = CompilationState::create_test_state_with_builtins();
    auto& ast_store = *state.ast_store_;

    auto value = create_numeric_literal(ast_store, "23", TSL);
    auto part_app_minus = create_partially_applied_minus(ast_store, value, TSL);
    auto type = create_type_reference(ast_store, &Float, TSL);

    auto outer = create_layer2_expression_testing(ast_store, {type, part_app_minus}, TSL);

    auto success = run_layer2(state, outer);

    CHECK(success);
    CHECK(*outer->type == Float);
}

TEST_CASE("Unary minus and type declaration") {
    auto [state, types] = CompilationState::create_test_state_with_builtins();
    auto& ast_store = *state.ast_store_;

    auto value = create_numeric_literal(ast_store, "23", TSL);
    auto minus_sign = create_minus_sign(ast_store, TSL);
    auto type = create_type_reference(ast_store, &Float, TSL);

    auto outer = create_layer2_expression_testing(ast_store, {type, minus_sign, value}, TSL);

    auto success = run_layer2(state, outer);

    CHECK(success);
    CHECK(*outer->type == Float);
}

TEST_CASE("Unary minus in parentheses and a type declaration") {
    auto [state, types] = CompilationState::create_test_state_with_builtins();
    auto& ast_store = *state.ast_store_;

    auto value = create_numeric_literal(ast_store, "23", TSL);
    auto minus_sign = create_minus_sign(ast_store, TSL);
    auto type = create_type_reference(ast_store, &Float, TSL);

    auto inner = create_layer2_expression_testing(ast_store, {minus_sign, value}, TSL);
    auto outer = create_layer2_expression_testing(ast_store, {type, inner}, TSL);

    auto success = run_layer2(state, outer);

    CHECK(success);
    CHECK(*outer->type == Float);
}

TEST_CASE("MutString (1 + 2)") {
    auto [state, _1] = CompilationState::create_test_state_with_builtins();
    auto& ast_store = *state.ast_store_;

    auto value1 = create_numeric_literal(ast_store, "1", TSL);
    auto value2 = create_numeric_literal(ast_store, "2", TSL);
    auto op = create_testing_binary_operator(ast_store, "+", &IntInt_to_Int, 500, TSL);
    auto op_ref = create_operator_reference(*state.ast_store_, op, TSL);
    auto type_specifier = create_type_reference(ast_store, &MutString, TSL);

    auto inner = create_layer2_expression_testing(ast_store, {value1, op_ref, value2}, TSL);
    auto expr = create_layer2_expression_testing(ast_store, {type_specifier, inner}, TSL);

    auto success = run_layer2(state, expr);

    CHECK(success);
    CHECK(*expr->type == MutString);
    CHECK(expr->expression_type == ExpressionType::call);
}