#include "doctest.h"

#include "mapsc/procedures/concretize.hh"
#include "mapsc/ast/ast_node.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/compiler_options.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Type concretizer should handle an Int Number") {
    Expression expr{
        ExpressionType::value,
        TEST_SOURCE_LOCATION,
        "34",
        &Number
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
        TEST_SOURCE_LOCATION,
        "3.4",
        &Number
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
        TEST_SOURCE_LOCATION,
        "3.4",
        &NumberLiteral
    };
    
    bool success = concretize(expr);

    CHECK(success);
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 3.4);
}

TEST_CASE("Concretizer should run variable substitution with a concrete type") {
    Expression value{ExpressionType::value, TSL, 1, &Int};
    Callable callable{&value, ""};
    Expression ref{ExpressionType::reference, TSL, &callable};
    ref.type = &Int;

    CHECK(concretize(ref));
    CHECK(ref == value);
} 

TEST_CASE("Concretizer should inline a nullary call with a concrete type") {
    Expression value{ExpressionType::value, TSL, 1, &Int};
    Callable callable{&value, ""};
    Expression call{ExpressionType::call, TSL, CallExpressionValue{&callable, {}}};
    call.type = &Int;

    CHECK(concretize(call));
    CHECK(call == value);
}

TEST_CASE("Concretizer should concretize the arguments to a call based on the callable type") {
    TypeStore types{};
    REQUIRE(types.empty());
    
    auto IntInt = types.get_function_type(Int, {&Int}, false);
    Expression value{ExpressionType::value, TSL, 1, IntInt};
    Callable const_Int{&value, "const_Int"};

    Expression arg{ExpressionType::value, TSL, "5", &Number};
    Expression call{ExpressionType::call, TSL, CallExpressionValue{&const_Int, {&arg}}};
    call.type = dynamic_cast<const FunctionType*>(const_Int.get_type())->return_type_;

    CHECK(concretize(call));
    CHECK(call != value);
    CHECK(*arg.type == Int);
    CHECK(std::holds_alternative<maps_Int>(arg.value));
    CHECK(std::get<maps_Int>(arg.value) == 5);
}

TEST_CASE("Concretizer should be able to concretize function calls based on arguments") {
    TypeStore types{};
    REQUIRE(types.empty());

    auto IntIntInt = types.get_function_type(Int, {&Int, &Int});
    Callable dummy_callable = Callable::testing_callable(IntIntInt);

    SUBCASE("Number -> Number -> Number into Int -> Int -> Int") {
        Expression arg1{ExpressionType::value, TSL, "12", &Number};
        Expression arg2{ExpressionType::value, TSL, "14", &Number};

        Expression call{ExpressionType::call, TSL, 
            CallExpressionValue{&dummy_callable, {&arg1, &arg2}}, &Int};

        CHECK(concretize(call));

        CHECK(*call.type == Int);
        CHECK(*arg1.type == Int);
        CHECK(*arg2.type == Int);
        CHECK(std::get<maps_Int>(arg1.value) == 12);
        CHECK(std::get<maps_Int>(arg2.value) == 14);
    }

    SUBCASE("Number -> Int -> Int into Int -> Int -> Int") {
        Expression arg1{ExpressionType::value, TSL, "12", &Number};
        Expression arg2{ExpressionType::value, TSL, 14, &Int};

        Expression call{ExpressionType::call, TSL, 
            CallExpressionValue{&dummy_callable, {&arg1, &arg2}}, &Int};

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

    auto IntIntInt = types.get_function_type(Float, {&Float, &Float});
    Callable dummy_callable = Callable::testing_callable(IntIntInt);

    SUBCASE("Number -> Int -> Float into Float -> Float -> Float") {
        Expression arg1{ExpressionType::value, TSL, "12.45", &Number};
        Expression arg2{ExpressionType::value, TSL, 148, &Int};

        Expression call{ExpressionType::call, TSL, 
            CallExpressionValue{&dummy_callable, {&arg1, &arg2}}, &Float};

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