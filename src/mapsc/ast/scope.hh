#ifndef __SCOPE_HH
#define __SCOPE_HH

#include <unordered_map>
#include <optional>
#include <variant>
#include <vector>
#include <cassert>

#include "mapsc/source.hh"
#include "mapsc/types/type.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/callable.hh"
#include "mapsc/ast/operator.hh"

namespace Maps {

// interface for visitors
// return false from a visit method to short circuit
template<class T>
concept AST_Visitor = requires(T t) {
    {t.visit_expression(std::declval<Expression*>())} -> std::convertible_to<bool>;
    {t.visit_statement(std::declval<Statement*>())} -> std::convertible_to<bool>;
    {t.visit_callable(std::declval<Callable*>())} -> std::convertible_to<bool>;
};

class AST_Store;

/**
 * Scopes contain names bound to callables
 * TODO: implement a constexpr hashmap that can be initialized in static memory 
 */
class Scope {
public:
    using const_iterator = std::vector<std::pair<std::string, Callable*>>::const_iterator;
    const_iterator begin() const { return identifiers_in_order_.begin(); }
    const_iterator end() const { return identifiers_in_order_.end(); }

    template<AST_Visitor T>
    bool walk_tree(T& visitor);

    template<AST_Visitor T>
    bool walk_expression(T visitor, Expression* expression);
    template<AST_Visitor T>
    bool walk_statement(T visitor, Statement* statement);
    template<AST_Visitor T>
    bool walk_callable(T visitor, Callable* callable);

    Scope() = default;

    bool identifier_exists(const std::string& name) const;
    std::optional<Callable*> get_identifier(const std::string& name) const;

    std::optional<Callable*> create_identifier(Callable* callable);

    // std::optional<Expression*> create_call_expression(
    //     const std::string& callee_name, std::vector<Expression*> args, SourceLocation location /*, expected return type?*/);
    // Expression* create_call_expression(Callable* callee, const std::vector<Expression*>& args, 
    //     SourceLocation location /*, expected return type?*/);

    std::vector<std::pair<std::string, Callable*>> identifiers_in_order_ = {};

private:
    std::unordered_map<std::string, Callable*> identifiers_;
};


template<AST_Visitor T>
bool Scope::walk_expression(T visitor, Expression* expression) {
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

        case ExpressionType::termed_expression:
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
bool Scope::walk_statement(T visitor, Statement* statement) {
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

        case StatementType::let:
            assert(false && "not implemented");
            return false;

        case StatementType::operator_definition:
        default:
            return true;
    }
}

template<AST_Visitor T>
bool Scope::walk_callable(T visitor, Callable* callable) {
    if (!visitor.visit_callable(callable))
        return false;

    if (Expression* const* expression = get_if<Expression*>(&callable->body)) {
        return walk_expression(visitor, *expression);
    } else if (Statement* const* statement = get_if<Statement*>(&callable->body)) {
        return walk_statement(visitor, *statement);
    }

    return true;
}

template<AST_Visitor T>
bool Scope::walk_tree(T& visitor) {
    for (auto [_1, callable]: identifiers_) {
        if (!walk_callable(visitor, callable))
            return false;
    }

    return true;
}


} // namespace AST

#endif