#ifndef __COMPILATION_STATE_HH
#define __COMPILATION_STATE_HH

#include <cassert>
#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "mapsc/pragma.hh"
#include "mapsc/builtins.hh"

#include "mapsc/types/type_store.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/callable.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/statement.hh" 

namespace Maps {

// interface for visitors
// return false from a visit method to short circuit
template<class T>
concept AST_Visitor = requires(T t) {
    {t.visit_expression(std::declval<Expression*>())} -> std::convertible_to<bool>;
    {t.visit_statement(std::declval<Statement*>())} -> std::convertible_to<bool>;
    {t.visit_callable(std::declval<Callable*>())} -> std::convertible_to<bool>;
};

class CompilationState {
public:
    struct SpecialCallables {
        BuiltinOperator* unary_minus;
        BuiltinOperator* binary_minus;
    };

    template<AST_Visitor T>
    bool walk_tree(T& visitor);

    template<AST_Visitor T>
    bool walk_expression(T visitor, Expression* expression);
    template<AST_Visitor T>
    bool walk_statement(T visitor, Statement* statement);
    template<AST_Visitor T>
    bool walk_callable(T visitor, Callable* callable);

    static std::tuple<CompilationState, std::unique_ptr<const Scope>, std::unique_ptr<TypeStore>> 
        create_test_state();

    CompilationState(const Scope* builtins, TypeStore* types,
        SpecialCallables specials = {&unary_minus_Int, &binary_minus_Int});

    [[nodiscard]] bool set_entry_point(Callable* entrypoint);
    [[nodiscard]] bool set_entry_point(std::string name);

    void declare_invalid() { is_valid = false; };

    bool is_valid = true;
    
    std::unique_ptr<Scope> globals_ = std::make_unique<Scope>();
    std::unique_ptr<AST_Store> ast_store_ = std::make_unique<AST_Store>();
    std::unique_ptr<PragmaStore> pragmas_ = std::make_unique<PragmaStore>();
    
    // container for top-level statements
    
    std::optional<Callable*> entry_point_ = std::nullopt;
    
    TypeStore* const types_;
    const Scope* const builtins_;
    const SpecialCallables special_callables_;

    // layer1 fills these with pointers to expressions that need work so that layer 2 doesn't
    // need to walk the tree to find them
    std::vector<Expression*> unresolved_identifiers_ = {};
    std::vector<Expression*> unresolved_type_identifiers_ = {};
    // these have to be dealt with before name resolution
    std::vector<Expression*> possible_binding_type_declarations_ = {};
    std::vector<Expression*> unparsed_termed_expressions_ = {};
};

template<AST_Visitor T>
bool CompilationState::walk_expression(T visitor, Expression* expression) {
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
bool CompilationState::walk_statement(T visitor, Statement* statement) {
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
bool CompilationState::walk_callable(T visitor, Callable* callable) {
    if (!visitor.visit_callable(callable))
        return false;

    if (Expression* const* expression = get_if<Expression*>(&callable->body_)) {
        return walk_expression(visitor, *expression);
    } else if (Statement* const* statement = get_if<Statement*>(&callable->body_)) {
        return walk_statement(visitor, *statement);
    }

    return true;
}

template<AST_Visitor T>
bool CompilationState::walk_tree(T& visitor) {
    for (auto [_1, callable]: globals_->identifiers_in_order_) {
        if (!walk_callable(visitor, callable))
            return false;
    }

    if (entry_point_ && !walk_callable(visitor, *entry_point_))
        return false;

    return true;
}

} // namespace Maps

#endif