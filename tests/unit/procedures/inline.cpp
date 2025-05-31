#include "doctest.h"

#include <array>

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/function_definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/procedures/inline.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/ast/ast_store.hh"

using namespace Maps;
using namespace std;

namespace {

std::tuple<AST_Store, TypeStore> setup() {
    return {
        AST_Store{},
        TypeStore{}
    };
}

} // namespace

#define COMMON_TESTS(function)\
\
REQUIRE(ref != value);\
\
SUBCASE("Both are Hole type") {\
    CHECK(function(ref, *def_body));\
    CHECK(ref == value);\
}\
\
SUBCASE("Should fail if declared types are incompatible") {\
    value.declared_type = &Int;\
    ref.declared_type = &Boolean;\
    \
    CHECK(!function(ref, *def_body));\
    CHECK(ref != value);\
}\
\
SUBCASE("Should pass if declared types and de facto types are all the same") {\
    value.declared_type = &Int;\
    ref.declared_type = &Int;\
    value.type = &Int;\
    ref.type = &Int;\
    \
    CHECK(function(ref, *def_body));\
    CHECK(ref == value);\
}

TEST_CASE("Should be able to substitute a reference to a value") {
    auto [ast_store, types] = setup();
    Expression value{ExpressionType::known_value, 1, TSL};
    auto def_body = create_nullary_function_definition(ast_store, types, &value, true, TSL);

    Expression ref{ExpressionType::reference, def_body->header_, TSL};

    COMMON_TESTS(substitute_value_reference);
}

TEST_CASE("Should be able to inline a nullary call to a value definition as if a reference") {
    auto [ast_store, types] = setup();
    Expression value{ExpressionType::known_value, 1, TSL};
    auto def_body = create_nullary_function_definition(ast_store, types, &value, true, TSL);

    Expression ref{ExpressionType::call, CallExpressionValue{def_body->header_, {}}, TSL};

    COMMON_TESTS(inline_call);
}

TEST_CASE("Should be able to inline a nullary call to a nullary pure function definition as if a reference") {
    auto [ast_store, types] = setup();
    
    Expression value{ExpressionType::known_value, 1, types.get_function_type(&Hole, {}, true), TSL};
    auto def_body = create_nullary_function_definition(ast_store, types, &value, true, TSL);
    Expression ref{ExpressionType::call, CallExpressionValue{def_body->header_, {}}, TSL};

    COMMON_TESTS(inline_call);
}

TEST_CASE("Should not be able to inline a nullary call to a nullary impure function definition as an expression") {
    auto [ast_store, types] = setup();

    Expression value{ExpressionType::known_value, 1, types.get_function_type(&Hole, {}, true), TSL};
    auto definition = create_nullary_function_definition(ast_store, types, &value, false, TSL);

    Expression ref{ExpressionType::call, CallExpressionValue{definition->header_, {}}, TSL};

    CHECK(!inline_call(ref, *definition));
    CHECK(ref != value);
}