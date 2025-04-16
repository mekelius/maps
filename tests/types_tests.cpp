#include "doctest.h"

#include "../src/lang/types.hh"
#include "../src/lang/ast.hh"

using AST::Expression, AST::ExpressionType, AST::Callable;

TEST_CASE("Operator types pass their priority correctly") {
    AST::Type op_type = AST::create_binary_operator_type(AST::Void, AST::Void, AST::Void, 77);
    CHECK(op_type.precedence() == 77);
}
