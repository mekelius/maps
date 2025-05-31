#include "doctest.h"

#include <array>

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/procedures/inline.hh"
#include "mapsc/types/type_store.hh"

using namespace Maps;
using namespace std;

#define COMMON_TESTS(function)\
\
REQUIRE(ref != value);\
\
SUBCASE("Both are Hole type") {\
    CHECK(function(ref, definition));\
    CHECK(ref == value);\
}\
\
SUBCASE("Should fail if declared types are incompatible") {\
    value.declared_type = &Int;\
    ref.declared_type = &Boolean;\
    \
    CHECK(!function(ref, definition));\
    CHECK(ref != value);\
}\
\
SUBCASE("Should pass if declared types and de facto types are all the same") {\
    value.declared_type = &Int;\
    ref.declared_type = &Int;\
    value.type = &Int;\
    ref.type = &Int;\
    \
    CHECK(function(ref, definition));\
    CHECK(ref == value);\
}

TEST_CASE("Should be able to substitute a reference to a value") {
    Expression value{ExpressionType::known_value, 1, TSL};
    DefinitionBody definition{&value, true, TSL};
    Expression ref{ExpressionType::reference, &definition, TSL};

    COMMON_TESTS(substitute_value_reference);
}

TEST_CASE("Should be able to inline a nullary call to a value definition as if a reference") {
    Expression value{ExpressionType::known_value, 1, TSL};
    DefinitionBody definition{&value, true, TSL};
    Expression ref{ExpressionType::call, CallExpressionValue{&definition, {}}, TSL};

    COMMON_TESTS(inline_call);
}

TEST_CASE("Should be able to inline a nullary call to a nullary pure function definition as if a reference") {
    TypeStore types{};
    
    Expression value{ExpressionType::known_value, 1, types.get_function_type(&Hole, array<const Type*, 0>{}, true), TSL};
    DefinitionBody definition{&value, true, TSL};
    Expression ref{ExpressionType::call, CallExpressionValue{&definition, {}}, TSL};

    COMMON_TESTS(inline_call);
}

TEST_CASE("Should not be able to inline a nullary call to a nullary pure function definition as an expression") {
    TypeStore types{};

    Expression value{ExpressionType::known_value, 1, types.get_function_type(&Hole, array<const Type*, 0>{}, false), TSL};
    DefinitionBody definition{&value, true, TSL};
    Expression ref{ExpressionType::call, CallExpressionValue{&definition, {}}, TSL};

    CHECK(!inline_call(ref, definition));
    CHECK(ref != value);
}