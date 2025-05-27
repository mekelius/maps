#include "doctest.h"

#include <variant>

#include "mapsc/source.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/procedures/type_check.hh"

using std::holds_alternative, std::get;
using namespace Maps;

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
        &Number,
        TSL,
    };

    CHECK(Number.cast_to(&Float, expr));
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 999);
}

TEST_CASE("Should be able to cast a known value into a constant function into that value") {
    auto [state, _1] = CompilationState::create_test_state_with_builtins();

    auto value = Expression::known_value(state, 23.3, TSL);

    CHECK(value->cast_to(state, &Int_to_Float));
    CHECK(*value->type == Int_to_Float);
}