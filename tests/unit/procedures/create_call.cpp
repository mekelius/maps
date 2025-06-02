#include "doctest.h"

#include "mapsc/procedures/create_call.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/external.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/types/type_defs.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Basics") {
    auto [state, _, types] = CompilationState::create_test_state();
    auto& ast_store = *state.ast_store_;

    auto callee_def = create_external(ast_store, "test", 
        types->get_function_type(&Float, {&Float, &Float}, false), TSL);

    SUBCASE ("Should do nothing if the types match") {
        auto value1 = create_known_value(state, static_cast<maps_Float>(1), TSL);
        auto value2 = create_known_value(state, static_cast<maps_Float>(2), TSL);

        std::vector<Expression*> args{value1, value2};

        auto [success, is_partial, is_done, return_type] = 
            check_and_coerce_args(state, callee_def, args, TSL);

        CHECK(success);
        CHECK(is_done);
        CHECK(*return_type == Float);
        CHECK(!is_partial);
        CHECK(*value1->type == Float);
        CHECK(*value2->type == Float);

        CHECK(args.size() == 2);
        CHECK(args.at(0) == value1);
        CHECK(args.at(1) == value2);
    }

    SUBCASE ("Should fill in missing args") {
        std::vector<Expression*> args{};

        auto [success, is_partial, is_done, return_type] = 
            check_and_coerce_args(state, callee_def, args, TSL);

        CHECK(success);
        CHECK(is_done);
        CHECK(is_partial);

        CHECK(args.size() == 2);

        auto arg1 = args.at(0);
        auto arg2 = args.at(1);

        CHECK(arg1->expression_type == ExpressionType::missing_arg);
        CHECK(*arg1->type == Float);
        CHECK(arg2->expression_type == ExpressionType::missing_arg);
        CHECK(*arg2->type == Float);
    }
}

// TEST_CASE("Creating an arg list should signal if there's work to be done later") {
//     // CHECK(!is_done);
    
// }

TEST_CASE("Creating an arg list should coerce args correctly") {
    auto [state, _, types] = CompilationState::create_test_state();
    auto& ast_store = *state.ast_store_;

    auto callee_def = create_external(ast_store, "test", 
        types->get_function_type(&Float, {&Float, &Float}, false), TSL);
    auto value1 = create_known_value(state, static_cast<maps_Int>(1), TSL);
    auto value2 = create_known_value(state, static_cast<maps_Int>(2), TSL);

    std::vector<Expression*> args{value1, value2};

    auto [success, is_partial, is_done, return_type] = check_and_coerce_args(state, callee_def, args, TSL);

    CHECK(success);
    CHECK(is_done);
    CHECK(!is_partial);
    CHECK(*return_type == Float);
    CHECK(*value1->type == Float);
    CHECK(*value2->type == Float);

    CHECK(args.size() == 2);
    CHECK(args.at(0) == value1);
    CHECK(args.at(1) == value2);
}

// TEST_CASE("Creating an arg list should copy references into values") {

// }

// TEST_CASE("Creating an arg list should add in missing args correctly") {
//     auto [state, _, types] = CompilationState::create_test_state();
//     auto& ast_store = *state.ast_store_;

//     auto callee_def = RT_Definition::external(ast_store, "test", 
//         types->get_function_type(&Float, {&Float, &Float}, false), TSL);

// }

// TEST_CASE("Creating an arg list should check the types of given missing args") {
    
// }

// TEST_CASE("Creating an arg list should try to deduce the types of args if Holes") {

// }

// TEST_CASE("Creating an arg list should reject if incompatible arg types") {
    
// }

// TEST_CASE("Creating an arg list should inline args if possible") {
    
// }

// TEST_CASE("Creating an arg list should insert runtime casts on unknown values") {
    
// }
