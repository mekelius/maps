#ifndef __AST_NODE_VISITOR_HH
#define __AST_NODE_VISITOR_HH

#include <concepts>

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/statement.hh"

namespace Maps {

// interface for visitors
// return false from a visit method to short circuit
template<class T>
concept AST_Visitor = requires(T t) {
    {t.visit_expression(std::declval<Expression*>())} -> std::convertible_to<bool>;
    {t.visit_statement(std::declval<Statement*>())} -> std::convertible_to<bool>;
    {t.visit_definition(std::declval<RT_Definition*>())} -> std::convertible_to<bool>;
};

template<AST_Visitor T>
bool walk_tree(T& visitor);

template<AST_Visitor T>
bool walk_expression(T visitor, Expression* expression);
template<AST_Visitor T>
bool walk_statement(T visitor, Statement* statement);
template<AST_Visitor T>
bool walk_definition(T visitor, RT_Definition* definition);

template<AST_Visitor T>
bool walk_expression(T visitor, Expression* expression) {
    if (!visitor.visit_expression(expression))
        return false;

    switch (expression->expression_type) {
        case ExpressionType::call:
            // can't visit the call target cause would get into infinite loops
            // !!! TODO: visit lambdas somehow
            for (Expression* arg: std::get<1>(expression->call_value())) {
                if (!walk_expression(visitor, arg))
                    return false;
            }
            return true;

        case ExpressionType::layer2_expression:
            for (Expression* sub_expression: expression->terms()) {
                if (!walk_expression(visitor, sub_expression))
                    return false;
            }
            return true;

        case ExpressionType::type_construct:
        case ExpressionType::type_argument:
            assert(false && "not implemented");
            return false;
        
        default:
            return true;
    }
}

template<AST_Visitor T>
bool walk_statement(T visitor, Statement* statement) {
    if (!visitor.visit_statement(statement))
        return false;

    switch (statement->statement_type) {
        case StatementType::assignment:
        case StatementType::expression_statement:
        case StatementType::return_:
            return walk_expression(visitor, get<Expression*>(statement->value));
        
        case StatementType::block:
            for (Statement* sub_statement: get<Block>(statement->value)) {
                if (!walk_statement(visitor, sub_statement))
                    return false;
            }
            return true;

        default:
            return true;
    }
}

template<AST_Visitor T>
bool walk_definition(T visitor, RT_Definition* definition) {
    if (!visitor.visit_definition(definition))
        return false;

    if (Expression* const* expression = get_if<Expression*>(&definition->body())) {
        return walk_expression(visitor, *expression);
    } else if (Statement* const* statement = get_if<Statement*>(&definition->body())) {
        return walk_statement(visitor, *statement);
    }

    return true;
}

template<AST_Visitor T>
bool walk_tree(T& visitor) {
    assert(false && "not implemented");
    // for (auto [_1, definition]: globals_.identifiers_in_order_) {
    //     if (!walk_definition(visitor, definition))
    //         return false;
    // }

    // return true;
}

} // namespace Maps

#endif