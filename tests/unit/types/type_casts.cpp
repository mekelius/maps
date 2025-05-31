#include "doctest.h"

#include <variant>

#include "mapsc/source.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/procedures/type_check.hh"
#include "mapsc/logging_options.hh"

using std::holds_alternative, std::get;
using namespace Maps;
using namespace std;

TEST_CASE("Should be able to cast a string into Float") {
    Expression expr{
        ExpressionType::known_value,
        "324.63",
        &String,
        TSL,
    };

    CHECK(String.cast_to(&Float, expr));
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 324.63);
}

TEST_CASE("Should be able to cast a Boolean into String") {
    Expression expr{
        ExpressionType::known_value,
        true,
        &Boolean,
        TSL,
    };

    CHECK(Boolean.cast_to(&String, expr));
    CHECK(*expr.type == String);
    CHECK(holds_alternative<std::string>(expr.value));
    CHECK(get<std::string>(expr.value) == "true");
}

TEST_CASE("Should be able to cast an Int into Float") {
    Expression expr{
        ExpressionType::known_value,
        324,
        &Int,
        TSL,
    };

    CHECK(Int.cast_to(&Float, expr));
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 324);
}

TEST_CASE("Should be able to cast a Float into String") {
    Expression expr{
        ExpressionType::known_value,
        544.963,
        &Float,
        TSL,
    };

    CHECK(Float.cast_to(&String, expr));
    CHECK(*expr.type == String);
    CHECK(holds_alternative<std::string>(expr.value));
    CHECK(get<std::string>(expr.value) == "544.963000");
}

TEST_CASE("Should be able to cast a Number with an int value into Float") {
    Expression expr{
        ExpressionType::known_value,
        "999",
        &NumberLiteral,
        TSL,
    };

    CHECK(NumberLiteral.cast_to(&Float, expr));
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 999);
}

TEST_CASE("Should be able to cast a known value into a constant function yielding that value") {
    auto [state, _1] = CompilationState::create_test_state_with_builtins();

    auto value = create_known_value(state, 23.3, TSL);

    CHECK(value->cast_to(state, &Int_to_Float));
    CHECK(*value->type == Int_to_Float);
}

TEST_CASE("Casting a known value into a function type with mathcing return type should produce a const lambda") {
    auto lock = LogOptions::set_global(LogLevel::debug_extra);

    auto [state, _0, types] = CompilationState::create_test_state();
    Scope scope{};

    const FunctionType* IntString = types->get_function_type(&String, array{&Int}, false);

    auto value = create_known_value(state, "qwe", &String, TSL);
    REQUIRE(value);

    auto const_lambda = (*value)->cast_to(state, IntString);

    CHECK(const_lambda);

    auto expr = *const_lambda;

    CHECK(expr->expression_type == ExpressionType::reference);
    CHECK(*expr->type == *IntString);
    CHECK(std::get<Expression*>(*expr->reference_value()->get_body_value())->string_value() == "qwe");
}