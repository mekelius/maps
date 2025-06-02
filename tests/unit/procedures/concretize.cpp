#include "doctest.h"

#include "mapsc/procedures/concretize.hh"
#include "mapsc/source.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/function_definition.hh"
#include "mapsc/ast/test_helpers/test_definition.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/let_definition.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Type concretizer should handle an Int Number") {
    auto [state, _, types] = CompilationState::create_test_state();

    Expression expr{
        ExpressionType::known_value,
        "34",
        &NumberLiteral,
        TSL,
    };
    
    bool success = concretize(state, expr);

    CHECK(success);
    CHECK(*expr.type == Int);
    CHECK(holds_alternative<maps_Int>(expr.value));
    CHECK(get<maps_Int>(expr.value) == 34);
}

TEST_CASE("Type concretizer should handle a Float Number") {
    auto [state, _, types] = CompilationState::create_test_state();

    Expression expr{
        ExpressionType::known_value,
        "3.4",
        &NumberLiteral,
        TSL,
    };
    
    bool success = concretize(state, expr);

    CHECK(success);
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 3.4);
}

TEST_CASE("Type concretizer should handle a Float NumberLiteral") {
    auto [state, _, types] = CompilationState::create_test_state();

    Expression expr{
        ExpressionType::known_value,
        "3.4",
        &NumberLiteral,
        TSL,
    };
    
    bool success = concretize(state, expr);

    CHECK(success);
    CHECK(*expr.type == Float);
    CHECK(holds_alternative<maps_Float>(expr.value));
    CHECK(get<maps_Float>(expr.value) == 3.4);
}

TEST_CASE("Concretizer should run variable substitution with a concrete type") {
    auto [state, _, types] = CompilationState::create_test_state();

    Expression value{ExpressionType::known_value, 1, &Int, TSL};
    auto definition = create_let_definition(*state.ast_store_, &value, TSL);
    Expression ref{ExpressionType::reference, definition->header_, TSL};
    ref.type = &Int;

    CHECK(concretize(state, ref));
    CHECK(ref == value);
} 

TEST_CASE("Concretizer should inline a nullary call with a concrete type") {
    auto [state, _, types] = CompilationState::create_test_state();

    Expression value{ExpressionType::known_value, 1, &Int, TSL};
    auto definition = create_nullary_function_definition(*state.ast_store_, *types, &value, true, TSL);
    Expression call{ExpressionType::call, CallExpressionValue{definition->header_, {}}, TSL};
    call.type = &Int;

    CHECK(concretize(state, call));
    CHECK(call == value);
}

TEST_CASE("Concretizer should concretize the arguments to a call based on the definition type") {
    auto [state, _, types] = CompilationState::create_test_state();
    REQUIRE(types->empty());
    
    auto IntInt = types->get_function_type(&Int, array{&Int}, false);
    Expression value{ExpressionType::known_value, 1, IntInt, TSL};

    auto const_Int = function_definition(state, "const_Int", &value, TSL);

    Expression arg{ExpressionType::known_value, "5", &NumberLiteral, TSL};
    Expression call{ExpressionType::call, CallExpressionValue{const_Int->header_, {&arg}}, TSL};
    call.type = dynamic_cast<const FunctionType*>(const_Int->get_type())->return_type();

    CHECK(concretize(state, call));
    CHECK(call != value);
    CHECK(*arg.type == Int);
    CHECK(std::holds_alternative<maps_Int>(arg.value));
    CHECK(std::get<maps_Int>(arg.value) == 5);
}

TEST_CASE("Concretizer should be able to concretize function calls based on arguments") {
    auto [state, _, types] = CompilationState::create_test_state();
    REQUIRE(types->empty());

    auto IntIntInt = types->get_function_type(&Int, array{&Int, &Int}, true);
    auto dummy_definition = create_let_definition(*state.ast_store_, IntIntInt, TSL)->header_;

    REQUIRE(*dummy_definition->get_type() == *IntIntInt);

    SUBCASE("Number -> Number -> Number into Int -> Int -> Int") {
        Expression arg1{ExpressionType::known_value, "12", &NumberLiteral, TSL};
        Expression arg2{ExpressionType::known_value, "14", &NumberLiteral, TSL};

        Expression call{ExpressionType::call,
            CallExpressionValue{dummy_definition, {&arg1, &arg2}}, &Int, TSL};

        CHECK(concretize(state, call));

        CHECK(*call.type == Int);
        CHECK(*arg1.type == Int);
        CHECK(*arg2.type == Int);
        CHECK(std::get<maps_Int>(arg1.value) == 12);
        CHECK(std::get<maps_Int>(arg2.value) == 14);
    }

    SUBCASE("Number -> Int -> Int into Int -> Int -> Int") {
        Expression arg1{ExpressionType::known_value, "12", &NumberLiteral, TSL};
        Expression arg2{ExpressionType::known_value, 14, &Int, TSL};

        Expression call{ExpressionType::call,
            CallExpressionValue{dummy_definition, {&arg1, &arg2}}, &Int, TSL};

        CHECK(concretize(state, call));

        CHECK(*call.type == Int);
        CHECK(*arg1.type == Int);
        CHECK(*arg2.type == Int);
        CHECK(std::get<maps_Int>(arg1.value) == 12);
        CHECK(std::get<maps_Int>(arg2.value) == 14);
    }

    SUBCASE("Number -> Int -> Int into Int -> Int -> Int") {
        Expression arg1{ExpressionType::known_value, "12", &NumberLiteral, TSL};
        Expression arg2{ExpressionType::known_value, 14, &Int, TSL};

        Expression call{ExpressionType::call,
            CallExpressionValue{dummy_definition, {&arg1, &arg2}}, &Int, TSL};

        CHECK(concretize(state, call));

        CHECK(*call.type == Int);
        CHECK(*arg1.type == Int);
        CHECK(*arg2.type == Int);
        CHECK(std::get<maps_Int>(arg1.value) == 12);
        CHECK(std::get<maps_Int>(arg2.value) == 14);
    }
}

TEST_CASE("Concretizer should be able to cast arguments up if needed") {
    auto [state, _, types] = CompilationState::create_test_state();
    REQUIRE(types->empty());

    auto IntIntInt = types->get_function_type(&Float, array{&Float, &Float}, true);
    auto dummy_definition = create_let_definition(*state.ast_store_, IntIntInt, TSL)->header_;

    SUBCASE("Number -> Int -> Float into Float -> Float -> Float") {
        Expression arg1{ExpressionType::known_value, "12.45", &NumberLiteral, TSL};
        Expression arg2{ExpressionType::known_value, 148, &Int, TSL};

        Expression call{ExpressionType::call, 
            CallExpressionValue{dummy_definition, {&arg1, &arg2}}, &Float, TSL};

        CHECK(concretize(state, call));

        CHECK(*call.type == Float);
        CHECK(*arg1.type == Float);
        CHECK(*arg2.type == Float);
        CHECK(std::holds_alternative<maps_Float>(arg1.value));
        CHECK(std::holds_alternative<maps_Float>(arg2.value));
        CHECK(std::get<maps_Float>(arg1.value) == 12.45);
        CHECK(std::get<maps_Float>(arg2.value) == 148);
    }
}
