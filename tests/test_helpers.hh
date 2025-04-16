#ifndef __TEST_HELPERS_HH
#define __TEST_HELPERS_HH

#include "../src/lang/ast.hh"

// jesus christ how horrifying
inline std::tuple<AST::Expression*, AST::Callable*> create_operator_helper(AST::AST& ast, 
    const std::string& op_string, unsigned int precedence = 500) {
    AST::Expression* op = ast.create_expression(AST::ExpressionType::builtin_operator, {0,0});
    op->value = op_string;
    op->type = AST::create_binary_operator_type(AST::Void, AST::Number, AST::Number, precedence);
    AST::Callable* op_callable = ast.create_callable({0,0});
    op_callable->body = op;
    AST::Expression* op_ref = ast.create_expression(AST::ExpressionType::operator_ref, {0,0});
    op_ref->value = op_callable;
    op_ref->type = op_callable->get_type();

    return {op_ref, op_callable};
}

#endif