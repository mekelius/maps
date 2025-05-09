#include "doctest.h"

#include "mapsc/ast/ast_node.hh"
#include "mapsc/procedures/inline.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/compiler_options.hh"

using namespace Maps;

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

TEST_CASE("Should be able to inline a nullary call to a value callable as if a reference") {
    Expression value{ExpressionType::value, TSL, 1};
    Callable callable{&value, ""};
    Expression ref{ExpressionType::call, TSL, CallExpressionValue{&callable, {}}};

    COMMON_TESTS(inline_call);
}

TEST_CASE("DEBUG_no_inline should make it fail") {
    auto opts = CompilerOptions::lock_for_this_thread({{CompilerOption::DEBUG_no_inline, "true"}});

    Expression value{ExpressionType::value, TSL, 1};
    Callable callable{&value, ""};
    Expression ref{ExpressionType::call, TSL, CallExpressionValue{&callable, {}}};

    #ifndef NDEBUG
    CHECK(!inline_call(ref, callable));
    CHECK(ref != value);
    #else
    CHECK(inline_call(ref, callable));
    CHECK(ref == value);
    #endif
}

TEST_CASE("Should be able to inline a nullary call to a nullary pure function callable as if a reference") {
    TypeStore types{};
    
    Expression value{ExpressionType::value, TSL, 1, types.get_function_type(Hole, {}, true)};
    Callable callable{&value, ""};
    Expression ref{ExpressionType::call, TSL, CallExpressionValue{&callable, {}}};

    COMMON_TESTS(inline_call);
}

TEST_CASE("Should not be able to inline a nullary call to a nullary pure function callable as an expression") {
    TypeStore types{};

    Expression value{ExpressionType::value, TSL, 1, types.get_function_type(Hole, {}, false)};
    Callable callable{&value, ""};
    Expression ref{ExpressionType::call, TSL, CallExpressionValue{&callable, {}}};

    CHECK(!inline_call(ref, callable));
    CHECK(ref != value);
}