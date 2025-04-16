#ifndef __TEST_HELPERS_HH
#define __TEST_HELPERS_HH

#include <sstream>

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

// takes a parsed binop expression tree and traverses it in preorder
void traverse_pre_order(AST::Expression* tree, std::ostream& output) {
    auto [op, lhs, rhs] = tree->binop_apply_value();

    output << std::get<AST::Expression*>(op->body)->string_value();

    if (lhs->expression_type == AST::ExpressionType::binary_operator_apply) {
        traverse_pre_order(lhs, output);
    } else {
        output << lhs->string_value();
    }

    if (rhs->expression_type == AST::ExpressionType::binary_operator_apply) {
        traverse_pre_order(rhs, output);
    } else {
        output << rhs->string_value();
    }
}

#endif