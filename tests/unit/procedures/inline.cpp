#include "doctest.h"

#include <array>

#include "mapsc/types/type_store.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/function_definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/procedures/inline.hh"

using namespace Maps;
using namespace std;

namespace {

std::tuple<CompilationState, std::shared_ptr<AST_Store>, std::unique_ptr<TypeStore>> setup() {
    auto [state, _, types] = CompilationState::create_test_state();
    
    return {
        state,
        state.ast_store_,
        std::move(types)
    };
}

} // namespace

#define COMMON_TESTS(function)\
\
REQUIRE(ref != *value);\
\
SUBCASE("Both are Hole type") {\
    CHECK(function(ref, *body));\
    CHECK(ref == *value);\
}\
\
SUBCASE("Should fail if declared types are incompatible") {\
    value->declared_type = &Int;\
    ref.declared_type = &Boolean;\
    \
    CHECK(!function(ref, *body));\
    CHECK(ref != *value);\
}\
\
SUBCASE("Should pass if declared types and de facto types are all the same") {\
    value->declared_type = &Int;\
    ref.declared_type = &Int;\
    value->type = &Int;\
    ref.type = &Int;\
    \
    CHECK(function(ref, *body));\
    CHECK(ref == *value);\
}

TEST_CASE("Should be able to substitute a reference to a value") {
    auto [state, ast_store, types] = setup();
    auto value = create_known_value(state, {1}, TSL);
    auto [header, body] = create_nullary_function_definition(*ast_store, *types, value, true, TSL);

    Expression ref{ExpressionType::reference, header, TSL};

    COMMON_TESTS(substitute_value_reference);
}

TEST_CASE("Should be able to inline a nullary call to a value definition as if a reference") {
    auto [state, ast_store, types] = setup();
    auto value = create_known_value(state, {1}, TSL);
    auto [header, body] = create_nullary_function_definition(*ast_store, *types, value, true, TSL);

    Expression ref{ExpressionType::call, CallExpressionValue{header, {}}, TSL};

    COMMON_TESTS(inline_call);
}

TEST_CASE("Should be able to inline a nullary call to a nullary pure function definition as if a reference") {
    auto [state, ast_store, types] = setup();
    
    auto value = *create_known_value(state, {1}, types->get_function_type(&Hole, {}, true), TSL);
    auto [header, body] = create_nullary_function_definition(*ast_store, *types, value, true, TSL);
    Expression ref{ExpressionType::call, CallExpressionValue{header, {}}, TSL};

    COMMON_TESTS(inline_call);
}

TEST_CASE("Should not be able to inline a nullary call to a nullary impure function definition as an expression") {
    auto [state, ast_store, types] = setup();

    auto value = *create_known_value(state, {1}, types->get_function_type(&Hole, {}, true), TSL);
    auto [header, body] = create_nullary_function_definition(*ast_store, *types, value, false, TSL);

    Expression ref{ExpressionType::call, CallExpressionValue{header, {}}, TSL};

    CHECK(!inline_call(ref, *body));
    CHECK(ref != *value);
}