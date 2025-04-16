#include "doctest.h"

#include "../src/lang/types.hh"
#include "../src/lang/ast.hh"

using AST::Expression, AST::ExpressionType, AST::Callable;

TEST_CASE("Callables should pass their type correctly") {
    const unsigned int PRECEDENCE = 756;
    AST::AST ast{};
    
    AST::Expression* op = ast.create_expression(AST::ExpressionType::builtin_operator, {0,0});
    op->type = AST::create_binary_operator_type(
        AST::Void, AST::Number, AST::Number, PRECEDENCE);

    AST::Callable* op_callable = ast.create_callable({0,0});
    op_callable->body = op;

    // AST::Expression* op_ref = ast.create_expression(AST::ExpressionType::operator_ref, {0,0});
    // op_ref->value = op_callable;
    
    CHECK(op->type.precedence() == PRECEDENCE);
}

TEST_CASE("Operator_ref:s should pass their type correctly") {
    const unsigned int PRECEDENCE = 243;
    AST::AST ast{};
    
    AST::Expression* op = ast.create_expression(AST::ExpressionType::builtin_operator, {0,0});
    op->type = AST::create_binary_operator_type(
        AST::Void, AST::Number, AST::Number, PRECEDENCE);

    AST::Callable* op_callable = ast.create_callable({0,0});
    op_callable->body = op;

    AST::Expression* op_ref = ast.create_expression(AST::ExpressionType::operator_ref, {0,0});
    op_ref->value = op_callable;
    
    CHECK(op_ref->callable_ref()->get_type().precedence() == PRECEDENCE);
}