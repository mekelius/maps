#include "doctest.h"

#include <sstream>

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/parser/layer2.hh"
#include "mapsc/logging_options.hh"
#include "mapsc/types/type_defs.hh"

using namespace Maps;
using namespace std;


TEST_CASE("Layer2 should handle type specifiers") {
    auto [state, _0, types] = CompilationState::create_test_state();

    auto type_specifier = Expression{ExpressionType::type_reference, &Int, TSL};
    auto value = Expression{ExpressionType::string_literal, "32", &String, TSL};

    SUBCASE("Int \"32\"") {
        auto expr = Expression{ExpressionType::termed_expression, 
            TermedExpressionValue{{&type_specifier, &value}, db_false}, TSL};

        run_layer2(state, &expr);

        CHECK(*expr.type == Int);
        CHECK(expr.expression_type == ExpressionType::known_value);

        CHECK(*value.type == Int);
        CHECK(holds_alternative<maps_Int>(value.value));
        CHECK(get<maps_Int>(value.value) == 32);
    }

    SUBCASE("Int \"32\" + 987") {
        auto op = RT_Operator{"+", External{}, 
            types->get_function_type(&Int, {&Int, &Int}, true),
            {Operator::Fixity::binary}, true, TSL};
        
        auto op_ref = Expression::operator_reference(*state.ast_store_, &op, TSL);
        auto rhs = Expression::numeric_literal(*state.ast_store_, "987", TSL);
        auto expr = Expression{ExpressionType::termed_expression, 
            TermedExpressionValue{{&type_specifier, &value, op_ref, rhs}, db_false}, TSL};

        run_layer2(state, &expr);

        CHECK(*expr.type == Int);
        CHECK(expr.expression_type == ExpressionType::call);

        auto [callee, args] = expr.call_value();

        CHECK(args.size() == 2);
        CHECK(callee == &op);
        CHECK(*value.type == Int);
        CHECK(holds_alternative<maps_Int>(value.value));
        CHECK(get<maps_Int>(value.value) == 32);
        CHECK(*args.at(0) == value);
    }
}

TEST_CASE("Should apply a type declaration") {
    auto [state, _0, types] = CompilationState::create_test_state();
    auto& ast_store = *state.ast_store_;

    auto value = Expression::numeric_literal(ast_store, "23", TSL);
    auto type = Expression::type_reference(ast_store, &Float, TSL);

    auto outer = Expression::termed(ast_store, {type, value}, TSL);

    auto success = run_layer2(state, outer);

    CHECK(success);
    CHECK(*outer->type == Float);
}

TEST_CASE("Partially applied minus and type declaration") {
    auto [state, types] = CompilationState::create_test_state_with_builtins();
    auto& ast_store = *state.ast_store_;

    auto value = Expression::numeric_literal(ast_store, "23", TSL);
    auto part_app_minus = Expression::partially_applied_minus(ast_store, value, TSL);
    auto type = Expression::type_reference(ast_store, &Float, TSL);

    auto outer = Expression::termed(ast_store, {type, part_app_minus}, TSL);

    auto success = run_layer2(state, outer);

    CHECK(success);
    CHECK(*outer->type == Float);
}

TEST_CASE("Unary minus and type declaration") {
    auto [state, types] = CompilationState::create_test_state_with_builtins();
    auto& ast_store = *state.ast_store_;

    auto value = Expression::numeric_literal(ast_store, "23", TSL);
    auto minus_sign = Expression::minus_sign(ast_store, TSL);
    auto type = Expression::type_reference(ast_store, &Float, TSL);

    auto outer = Expression::termed(ast_store, {type, minus_sign, value}, TSL);

    auto success = run_layer2(state, outer);

    CHECK(success);
    CHECK(*outer->type == Float);
}

TEST_CASE("Unary minus in parentheses and a type declaration") {
    auto [state, types] = CompilationState::create_test_state_with_builtins();
    auto& ast_store = *state.ast_store_;

    auto value = Expression::numeric_literal(ast_store, "23", TSL);
    auto minus_sign = Expression::minus_sign(ast_store, TSL);
    auto type = Expression::type_reference(ast_store, &Float, TSL);

    auto inner = Expression::termed(ast_store, {minus_sign, value}, TSL);
    auto outer = Expression::termed(ast_store, {type, inner}, TSL);

    auto success = run_layer2(state, outer);

    CHECK(success);
    CHECK(*outer->type == Float);
}

TEST_CASE("MutString (1 + 2)") {
    auto options = LogOptions::Lock::global();
    options.options_->set_loglevel(LogLevel::debug_extra);

    auto [state, _1] = CompilationState::create_test_state_with_builtins();
    auto& ast_store = *state.ast_store_;

    auto value1 = Expression::numeric_literal(ast_store, "1", TSL);
    auto value2 = Expression::numeric_literal(ast_store, "2", TSL);
    auto op = RT_Operator{"+", External{}, &IntInt_to_Int, {Operator::Fixity::binary}, true, TSL};
    auto op_ref = Expression::operator_reference(*state.ast_store_, &op, TSL);
    auto type_specifier = Expression::type_reference(ast_store, &MutString, TSL);

    auto inner = Expression::termed(ast_store, {value1, op_ref, value2}, TSL);
    auto expr = Expression::termed(ast_store, {type_specifier, inner}, TSL);

    auto success = run_layer2(state, expr);

    ReverseParser::Options rp_options{};
    rp_options.include_all_types = true;
    rp_options.debug_node_types = true;
    ReverseParser{&std::cerr, rp_options} << *expr << "\n";

    CHECK(success);
    CHECK(*expr->type == MutString);
    CHECK(expr->expression_type == ExpressionType::call);
}