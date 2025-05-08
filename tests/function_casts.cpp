#include <doctest.h>

#include "mapsc/ast/ast_node.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/types/type_checking.hh"

using namespace Maps;

constexpr auto TSL = TEST_SOURCE_LOCATION;



// TEST_CASE("Concretizer should be able to concretize  function calls based on arguments") {
//     TypeStore types{};
//     REQUIRE(types.empty());

//     auto IntIntInt = types.get_function_type(Int, {&Int, &Int});
//     Callable dummy_callable = Callable::testing_callable(IntIntInt);

//     SUBCASE("Number -> Number -> Number into Int -> Int -> Int") {
//         Expression arg1{ExpressionType::value, TSL, "12", &Number};
//         Expression arg2{ExpressionType::value, TSL, "14", &Number};

//         Expression call{ExpressionType::call, TSL, 
//             CallExpressionValue{&dummy_callable, {&arg1, &arg2}}, &Number};

//         CHECK(TypeConcretizer{}.concretize_call(call));

//         CHECK(*call.type == *IntIntInt);
//         CHECK(*arg1.type == Int);
//         CHECK(*arg2.type == Int);
//     }
// }

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

        CHECK(TypeConcretizer{}.concretize_call(call));

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