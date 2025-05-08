#include "doctest.h"

#include <variant>

#include "mapsc/ast/ast_node.hh"
#include "mapsc/types/type_checking.hh"

using std::holds_alternative, std::get;
using namespace Maps;

TEST_CASE("Should be able to cast a string into Float") {
    Expression expr{
        ExpressionType::value,
        TEST_SOURCE_LOCATION,
        "324.63",
        &String
    };

    CHECK(String.cast_to(&Float, expr));
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 324.63);
}

TEST_CASE("Should be able to cast a Boolean into String") {
    Expression expr{
        ExpressionType::value,
        TEST_SOURCE_LOCATION,
        true,
        &Boolean
    };

    CHECK(Boolean.cast_to(&String, expr));
    CHECK(*expr.type == String);
    CHECK(holds_alternative<std::string>(expr.value));
    CHECK(get<std::string>(expr.value) == "true");
}

TEST_CASE("Should be able to cast an Int into Float") {
    Expression expr{
        ExpressionType::value,
        TEST_SOURCE_LOCATION,
        324,
        &Int
    };

    CHECK(Int.cast_to(&Float, expr));
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 324);
}

TEST_CASE("Should be able to cast a Float into String") {
    Expression expr{
        ExpressionType::value,
        TEST_SOURCE_LOCATION,
        544.963,
        &Float
    };

    CHECK(Float.cast_to(&String, expr));
    CHECK(*expr.type == String);
    CHECK(holds_alternative<std::string>(expr.value));
    CHECK(get<std::string>(expr.value) == "544.963000");
}

TEST_CASE("Should be able to cast a Number with an int value into Float") {
    Expression expr{
        ExpressionType::value,
        TEST_SOURCE_LOCATION,
        "999",
        &Number
    };

    CHECK(Number.cast_to(&Float, expr));
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 999);
}
