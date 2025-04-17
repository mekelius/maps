#include "doctest.h"

#include "../src/lang/types.hh"
#include "../src/lang/ast.hh"

using AST::Expression, AST::ExpressionType, AST::Callable, AST::BuiltinType;

TEST_CASE("Callables should pass their type correctly") {
    const unsigned int PRECEDENCE = 756;
    AST::AST ast{};
    
    AST::Callable* op = ast.create_builtin(BuiltinType::builtin_operator, "*", 
        AST::create_binary_operator_type(AST::Void, AST::Number, AST::Number, PRECEDENCE));

    CHECK(op->get_type().precedence() == PRECEDENCE);
}

TEST_CASE("Operator_ref:s should pass their type correctly") {
    const unsigned int PRECEDENCE = 243;
    AST::AST ast{};
    
    AST::Callable* op = ast.create_builtin(BuiltinType::builtin_operator, "*", 
        AST::create_binary_operator_type(AST::Void, AST::Number, AST::Number, PRECEDENCE));

    AST::Expression* op_ref = ast.create_operator_ref(op, {0,0});
    
    CHECK(op_ref->type.precedence() == PRECEDENCE);
}