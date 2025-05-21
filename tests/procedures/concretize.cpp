#include "doctest.h"

#include "mapsc/procedures/concretize.hh"
#include "mapsc/source.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/types/type_store.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Type concretizer should handle an Int Number") {
    Expression expr{
        ExpressionType::value,
        "34",
        &Number,
        TEST_SOURCE_LOCATION
    };
    
    bool success = concretize(expr);

    CHECK(success);
    CHECK(*expr.type == Int);
    CHECK(holds_alternative<maps_Int>(expr.value));
    CHECK(get<maps_Int>(expr.value) == 34);
}

TEST_CASE("Type concretizer should handle a Float Number") {
    Expression expr{
        ExpressionType::value,
        "3.4",
        &Number,
        TEST_SOURCE_LOCATION,
    };
    
    bool success = concretize(expr);

    CHECK(success);
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 3.4);
}

TEST_CASE("Type concretizer should handle a Float NumberLiteral") {
    Expression expr{
        ExpressionType::value,
        "3.4",
        &NumberLiteral,
        TEST_SOURCE_LOCATION,
    };
    
    bool success = concretize(expr);

    CHECK(success);
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 3.4);
}

TEST_CASE("Concretizer should run variable substitution with a concrete type") {
    Expression value{ExpressionType::value, 1, &Int, TSL};
    RT_Definition definition{&value, TSL};
    Expression ref{ExpressionType::reference, &definition, TSL};
    ref.type = &Int;

    CHECK(concretize(ref));
    CHECK(ref == value);
} 

TEST_CASE("Concretizer should inline a nullary call with a concrete type") {
    Expression value{ExpressionType::value, 1, &Int, TSL};
    RT_Definition definition{&value, TSL};
    Expression call{ExpressionType::call, CallExpressionValue{&definition, {}}, TSL};
    call.type = &Int;

    CHECK(concretize(call));
    CHECK(call == value);
}

TEST_CASE("Concretizer should concretize the arguments to a call based on the definition type") {
    TypeStore types{};
    REQUIRE(types.empty());
    
    auto IntInt = types.get_function_type(Int, {&Int}, false);
    Expression value{ExpressionType::value, 1, IntInt, TSL};
    RT_Definition const_Int{"const_Int", &value, TSL};

    Expression arg{ExpressionType::value, "5", &Number, TSL};
    Expression call{ExpressionType::call, CallExpressionValue{&const_Int, {&arg}}, TSL};
    call.type = dynamic_cast<const FunctionType*>(const_Int.get_type())->return_type();

    CHECK(concretize(call));
    CHECK(call != value);
    CHECK(*arg.type == Int);
    CHECK(std::holds_alternative<maps_Int>(arg.value));
    CHECK(std::get<maps_Int>(arg.value) == 5);
}

TEST_CASE("Concretizer should be able to concretize function calls based on arguments") {
    TypeStore types{};
    REQUIRE(types.empty());

    auto IntIntInt = types.get_function_type(Int, {&Int, &Int}, true);
    RT_Definition dummy_definition = RT_Definition::testing_definition(IntIntInt);

    REQUIRE(*dummy_definition.get_type() == *IntIntInt);

    SUBCASE("Number -> Number -> Number into Int -> Int -> Int") {
        Expression arg1{ExpressionType::value, "12", &Number, TSL};
        Expression arg2{ExpressionType::value, "14", &Number, TSL};

        Expression call{ExpressionType::call,
            CallExpressionValue{&dummy_definition, {&arg1, &arg2}}, &Int, TSL};

        CHECK(concretize(call));

        CHECK(*call.type == Int);
        CHECK(*arg1.type == Int);
        CHECK(*arg2.type == Int);
        CHECK(std::get<maps_Int>(arg1.value) == 12);
        CHECK(std::get<maps_Int>(arg2.value) == 14);
    }

    SUBCASE("Number -> Int -> Int into Int -> Int -> Int") {
        Expression arg1{ExpressionType::value, "12", &Number, TSL};
        Expression arg2{ExpressionType::value, 14, &Int, TSL};

        Expression call{ExpressionType::call,
            CallExpressionValue{&dummy_definition, {&arg1, &arg2}}, &Int, TSL};

        CHECK(concretize(call));

        CHECK(*call.type == Int);
        CHECK(*arg1.type == Int);
        CHECK(*arg2.type == Int);
        CHECK(std::get<maps_Int>(arg1.value) == 12);
        CHECK(std::get<maps_Int>(arg2.value) == 14);
    }
}

TEST_CASE("Concretizer should be able to cast arguments up if needed") {
    TypeStore types{};
    REQUIRE(types.empty());

    auto IntIntInt = types.get_function_type(Float, {&Float, &Float}, true);
    RT_Definition dummy_definition = RT_Definition::testing_definition(IntIntInt);

    SUBCASE("Number -> Int -> Float into Float -> Float -> Float") {
        Expression arg1{ExpressionType::value, "12.45", &Number, TSL};
        Expression arg2{ExpressionType::value, 148, &Int, TSL};

        Expression call{ExpressionType::call, 
            CallExpressionValue{&dummy_definition, {&arg1, &arg2}}, &Float, TSL};

        CHECK(concretize(call));

        CHECK(*call.type == Float);
        CHECK(*arg1.type == Float);
        CHECK(*arg2.type == Float);
        CHECK(std::holds_alternative<maps_Float>(arg1.value));
        CHECK(std::holds_alternative<maps_Float>(arg2.value));
        CHECK(std::get<maps_Float>(arg1.value) == 12.45);
        CHECK(std::get<maps_Float>(arg2.value) == 148);
    }
}
