#include "doctest.h"

#include "mapsc/procedures/create_call.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/types/type_defs.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Creating an arg list should signal if there's work to be done later") {
    
}

TEST_CASE("Creating an arg list should coerce args correctly") {
    auto [state, _, types] = CompilationState::create_test_state();
    auto& ast_store = *state.ast_store_;

    auto callee_def = RT_Definition::external(ast_store, "test", 
        types->get_function_type(&Float, {&Float, &Float}, false), TSL);
    auto value1 = Expression::known_value(ast_store, static_cast<maps_Int>(1), TSL);
    auto value2 = Expression::known_value(ast_store, static_cast<maps_Int>(2), TSL);

    auto callee_ref = Expression::reference(ast_store, callee_def, TSL);

    std::vector<Expression*> args{value1, value2};

    auto [success, is_partial] = check_and_coerce_args(ast_store, callee_def, args, TSL);

    CHECK(success);
    CHECK(!is_partial);
    CHECK(*value1->type == Float);
    CHECK(*value2->type == Float);

    CHECK(args.size() == 2);
    CHECK(args.at(0) == value1);
    CHECK(args.at(1) == value2);
}

TEST_CASE("Creating an arg list should copy references into values") {

}

TEST_CASE("Creating an arg list should add in missing args correctly") {

}

TEST_CASE("Creating an arg list should check the types of given missing args") {
    
}

TEST_CASE("Creating an arg list should try to deduce the types of args if Holes") {

}

TEST_CASE("Creating an arg list should reject if incompatible arg types") {
    
}

TEST_CASE("Creating an arg list should inline args if possible") {
    
}

TEST_CASE("Creating an arg list should insert runtime casts on unknown values") {
    
}
