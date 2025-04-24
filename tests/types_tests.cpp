#include "doctest.h"

#include "../src/lang/type.hh"
#include "../src/lang/ast.hh"

using Maps::Expression, Maps::ExpressionType, Maps::Callable;

TEST_CASE("Operator types pass their priority correctly") {
    Maps::TypeRegistry types;
    const Maps::Type* op_type = types.get_binary_operator_type(Maps::Void, Maps::Void, Maps::Void, 77);
    CHECK(op_type->precedence() == 77);
}
