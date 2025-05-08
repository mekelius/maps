#include "doctest.h"

#include "mapsc/ast/ast_node.hh"
#include "mapsc/inline.hh"

using namespace Maps;
constexpr auto TSL = TEST_SOURCE_LOCATION;

#define COMMON_TESTS(function)\
\
REQUIRE(ref != value);\
\
SUBCASE("Both are Hole type") {\
    CHECK(function(ref, callable));\
    CHECK(ref == value);\
}\
\
SUBCASE("Should fail if declared types are incompatible") {\
    value.declared_type = &Int;\
    ref.declared_type = &Boolean;\
    \
    CHECK(!function(ref, callable));\
    CHECK(ref != value);\
}\
\
SUBCASE("Should pass if declared types and de facto types are all the same") {\
    value.declared_type = &Int;\
    ref.declared_type = &Int;\
    value.type = &Int;\
    ref.type = &Int;\
    \
    CHECK(function(ref, callable));\
    CHECK(ref == value);\
}

TEST_CASE("Should be able to substitute a reference to a value") {
    Expression value{ExpressionType::value, TSL, 1};
    Callable callable{&value, ""};
    Expression ref{ExpressionType::reference, TSL, &callable};

    COMMON_TESTS(substitute_value_reference);
}

TEST_CASE("Should be able to inline a nullary call as if a value") {
    Expression value{ExpressionType::value, TSL, 1};
    Callable callable{&value, ""};
    Expression ref{ExpressionType::call, TSL, CallExpressionValue{&callable, {}}};

    COMMON_TESTS(inline_call);
}