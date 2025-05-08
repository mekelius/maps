#include "doctest.h"

#include "mapsc/procedures/concretize.hh"
#include "mapsc/ast/ast_node.hh"
#include "mapsc/types/type_store.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Type concretizer should handle an Int Number") {
    Expression expr{
        ExpressionType::value,
        TEST_SOURCE_LOCATION,
        "34",
        &Number
    };
    
    bool success = concretize_expression(expr);

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
    
    bool success = concretize_expression(expr);

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
    
    bool success = concretize_expression(expr);

    CHECK(success);
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 3.4);
}


TEST_CASE("Concretizer should run variable substitution with a concrete type") {
    Expression value{ExpressionType::value, TSL, 1, &Int};
    Callable callable{&value, ""};
    Expression ref{ExpressionType::reference, TSL, &callable};

    CHECK(concretize_expression(ref));
    CHECK(ref == value);
} 

TEST_CASE("Concretizer should inline a nullary call with a concrete type") {
    Expression value{ExpressionType::value, TSL, 1, &Int};
    Callable callable{&value, ""};
    Expression call{ExpressionType::call, TSL, CallExpressionValue{&callable, {}}};

    CHECK(concretize_expression(call));
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

    CHECK(concretize_expression(call));
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
            CallExpressionValue{&dummy_callable, {&arg1, &arg2}}, &Number};

        CHECK(concretize_expression(call));

        CHECK(*call.type == *IntIntInt);
        CHECK(*arg1.type == Int);
        CHECK(*arg2.type == Int);
    }

    // SUBCASE("Number -> Int -> Number into Int -> Int -> Int") {
    //     Expression arg1{ExpressionType::value, TSL, "12", &Number};
    //     Expression arg2{ExpressionType::value, TSL, 14, &Int};

    //     Expression call{ExpressionType::call, TSL, "", types.get_function_type(Number, {&Number, &Int})};

    //     CHECK(TypeConcretizer{}.concretize_call(call));

    //     CHECK(*call.type == *types.get_function_type(Int, {&Int, &Int}));
    //     CHECK(*arg1.type == Int);
    //     CHECK(*arg2.type == Int);
    // }

    // SUBCASE("Number -> Number -> Number into Float -> Float -> Float") {
    //     TypeRegistry types{};

    //     Expression arg1{ExpressionType::value, TSL, "12.123", &Number};
    //     Expression arg2{ExpressionType::value, TSL, "14.2", &Number};

    //     Expression call{ExpressionType::call, TSL, CallExpressionValue{&dummy_callable, {&arg1, &arg2}}, 
    //         types.get_function_type(Number, {&Number, &Number})};

    //     CHECK(TypeConcretizer{}.concretize_call(call));

    //     CHECK(*call.type == *types.get_function_type(Int, {&Int, &Int}));
    //     CHECK(*arg1.type == Int);
    //     CHECK(*arg2.type == Int);
    // }

    // // NOTE: at the moment we are willing to accept a compromize for 0.1, the types are concretized into
    // // Float -> Float -> Float, since we can't yet derive Float -> Int -> Float
    // SUBCASE("Number -> Number -> Number with mixed Int and Float args into Float -> Float -> Float") {
    //     Expression arg1{ExpressionType::value, TSL, "12", &Number};
    //     Expression arg2{ExpressionType::value, TSL, "14.2", &Number};

    //     Expression call{ExpressionType::call, TSL, "", types.get_function_type(Number, {&Number, &Number})};

    //     CHECK(TypeConcretizer{}.concretize_call(call));

    //     CHECK(*call.type == *types.get_function_type(Float, {&Float, &Float}));
    //     CHECK(*arg1.type == Int);
    //     CHECK(*arg2.type == Int);
    // }
}