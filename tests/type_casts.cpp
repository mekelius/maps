#include "doctest.h"

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
    CHECK(holds_alternative<float_t>(expr.value));
    CHECK(get<float_t>(expr.value) == 324.63);
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
    CHECK(holds_alternative<float_t>(expr.value));
    CHECK(get<float_t>(expr.value) == 324);
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
    CHECK(holds_alternative<float_t>(expr.value));
    CHECK(get<float_t>(expr.value) == 999);
}

TEST_CASE("Type concretizer should handle an Int Number") {
    Expression expr{
        ExpressionType::value,
        TEST_SOURCE_LOCATION,
        "34",
        &Number
    };
    
    bool success = TypeConcretizer{}.concretize_value(expr);

    CHECK(success);
    CHECK(*expr.type == Int);
    CHECK(holds_alternative<int_t>(expr.value));
    CHECK(get<int_t>(expr.value) == 34);
}

TEST_CASE("Type concretizer should handle a Float Number") {
    Expression expr{
        ExpressionType::value,
        TEST_SOURCE_LOCATION,
        "3.4",
        &Number
    };
    
    bool success = TypeConcretizer{}.concretize_value(expr);

    CHECK(success);
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<float_t>(expr.value));
    CHECK(get<float_t>(expr.value) == 3.4);
}

TEST_CASE("Type concretizer should handle a Float NumberLiteral") {
    Expression expr{
        ExpressionType::value,
        TEST_SOURCE_LOCATION,
        "3.4",
        &NumberLiteral
    };
    
    bool success = TypeConcretizer{}.concretize_value(expr);

    CHECK(success);
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<float_t>(expr.value));
    CHECK(get<float_t>(expr.value) == 3.4);
}