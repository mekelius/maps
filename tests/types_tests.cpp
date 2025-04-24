#include "doctest.h"

#include "../src/lang/types.hh"
#include "../src/lang/ast.hh"

using Maps::Expression, Maps::ExpressionType, Maps::Callable;

TEST_CASE("Operator types pass their priority correctly") {
    Maps::Type op_type = Maps::create_binary_operator_type(Maps::Void, Maps::Void, Maps::Void, 77);
    CHECK(op_type.precedence() == 77);
}
