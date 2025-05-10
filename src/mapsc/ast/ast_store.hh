#ifndef __AST_HH
#define __AST_HH

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include <cassert>

#include "mapsc/types/type.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/builtin.hh"

#include "mapsc/ast/ast_node.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/scope.hh"

namespace Maps {

// interface for visitors
// return false from a visit method to short circuit
template<class T>
concept AST_Visitor = requires(T t) {
    {t.visit_expression(std::declval<Expression*>())} -> std::convertible_to<bool>;
    {t.visit_statement(std::declval<Statement*>())} -> std::convertible_to<bool>;
    {t.visit_callable(std::declval<Callable*>())} -> std::convertible_to<bool>;
};

class AST_Store {
public:
    using const_iterator = Scope::const_iterator;

    AST_Store();
    [[nodiscard]] bool init_builtins();

    void set_root(CallableBody root);
    void declare_invalid() { is_valid = false; };
    
    bool empty() const;
    size_t size() const;

    // ----- WALKING TREE NODE BY NODE -----

    template<AST_Visitor T>
    bool walk_tree(T& visitor);

    template<AST_Visitor T>
    bool walk_expression(T visitor, Expression* expression);
    template<AST_Visitor T>
    bool walk_statement(T visitor, Statement* statement);
    template<AST_Visitor T>
    bool walk_callable(T visitor, Callable* callable);

    // ----- ITERATING THROUGH CALLABLES -----

    // This will at some point be replaced with something that supports nested scopes
    const_iterator begin() const { return globals_->begin(); }
    const_iterator end() const { return globals_->end(); }
    
    void delete_expression(Expression* expression);
    void delete_expression_recursive(Expression* expression);

    void delete_statement(Statement* statement);
    void delete_statement_recursive(Statement* statement);

    //  ----- CREATING OTHER THINGS -----
    Statement* create_statement(StatementType statement_type, SourceLocation location);
    
    // automatically creates an identifier and a global callable for the builtin
    Callable* create_builtin(const std::string& name, const Type& type);
    Callable* create_builtin_binary_operator(const std::string& name, const Type& type, 
        Precedence precedence, Associativity Associativity = Associativity::left);
    Callable* create_builtin_unary_operator(const std::string& name, const Type& type, 
        UnaryFixity fixity = UnaryFixity::prefix);

    // ----- FIELDS -----

    bool is_valid = true;
    // container for top-level statements
    Callable* root_;

    std::unique_ptr<Scope> globals_ = std::make_unique<Scope>(this);
    std::unique_ptr<Scope> builtins_scope_ = std::make_unique<Scope>(this);

    std::unique_ptr<TypeStore> types_ = std::make_unique<TypeStore>();

    // layer1 fills these with pointers to expressions that need work so that layer 2 doesn't
    // need to walk the tree to find them
    std::vector<Expression*> unresolved_identifiers_ = {};
    std::vector<Expression*> unresolved_type_identifiers_ = {};
    // these have to be dealt with before name resolution
    std::vector<Expression*> possible_binding_type_declarations_ = {};
    std::vector<Expression*> unparsed_termed_expressions_ = {};

    Expression* allocate_expression(const Expression&& expr);
private:
    friend Scope; // scope is allowed to call create_expression directly to create call expressions

    Callable* create_callable(CallableBody body, const std::string& name, 
        std::optional<SourceLocation> location = std::nullopt);
    Callable* create_callable(const std::string& name, SourceLocation location);

    Operator* create_operator(const Operator&& operator_props);

    // currently these guys, once created, stay in memory forever
    // we could create a way to sweep them by having some sort of "alive"-flag
    // or maybe "DeletedStatement" statement type
    // probably won't need that until we do the interpreter
    // TODO: move from vector of unique_ptrs to unique_ptr of vectors
    std::vector<std::unique_ptr<Statement>> statements_ = {};
    std::vector<std::unique_ptr<Expression>> expressions_ = {};
    std::vector<std::unique_ptr<Builtin>> builtins_ = {};
    std::vector<std::unique_ptr<Callable>> callables_ = {};
    std::vector<std::unique_ptr<Operator>> operators_ = {};
};

template<AST_Visitor T>
bool AST_Store::walk_expression(T visitor, Expression* expression) {
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
bool AST_Store::walk_statement(T visitor, Statement* statement) {
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
bool AST_Store::walk_callable(T visitor, Callable* callable) {
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
bool AST_Store::walk_tree(T& visitor) {
    for (auto [_1, callable]: globals_->identifiers_) {
        if (!walk_callable(visitor, callable))
            return false;
    }

    return walk_callable(visitor, root_);
}

} // namespace Maps

#endif