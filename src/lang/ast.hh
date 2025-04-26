#ifndef __AST_HH
#define __AST_HH

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include <cassert>

#include "type.hh"
#include "type_defs.hh"
#include "ast_node.hh"
#include "scope.hh"

namespace Maps {

template<class T>
concept AST_Visitor = requires(T t) {
    {t.visit_expression(std::declval<Expression*>())};
    {t.visit_statement(std::declval<Statement*>())};
    {t.visit_callable(std::declval<Callable*>())};
};

class AST {
public:
    AST();
    void set_root(CallableBody root);
    void declare_invalid() { is_valid = false; };

    template<AST_Visitor T>
    void visit_nodes(T visitor);

    template<AST_Visitor T>
    void walk_expression(T visitor, Expression* expression);
    template<AST_Visitor T>
    void walk_statement(T visitor, Statement* statement);
    template<AST_Visitor T>
    void walk_callable(T visitor, Callable* callable);

    // ----- CREATING (AND DELETING) EXPRESSIONS -----
    Expression* create_string_literal(const std::string& value, SourceLocation location);
    Expression* create_numeric_literal(const std::string& value, SourceLocation location);
    
    // These automatically add the identifier into unresolved list as a convenience
    Expression* create_identifier_expression(const std::string& value, SourceLocation location);
    Expression* create_type_identifier_expression(const std::string& value, SourceLocation location);
    Expression* create_operator_expression(const std::string& value, SourceLocation location);
    Expression* create_type_operator_expression(const std::string& value, SourceLocation location);

    Expression* create_termed_expression(std::vector<Expression*>&& terms, SourceLocation location);

    std::optional<Expression*> create_type_operator_ref(const std::string& name, SourceLocation location);

    Expression* create_type_reference(const Type* type, SourceLocation location);
    std::optional<Expression*> create_operator_ref(const std::string& name, SourceLocation location);
    Expression* create_operator_ref(Callable* callable, SourceLocation location);

    // valueless expression types are tie, empty, syntax_error and not_implemented
    Expression* create_valueless_expression(ExpressionType expression_type, SourceLocation location);
    Expression* create_missing_argument(const Type& type, SourceLocation location);
    
    void delete_expression(Expression* expression);

    //  ----- CREATING OTHER THINGS -----
    Statement* create_statement(StatementType statement_type, SourceLocation location);
    
    // automatically creates an identifier and a global callable for the builtin
    Callable* create_builtin(const std::string& name, const Type& type);
    Callable* create_builtin_binary_operator(const std::string& name, const Type& type, Precedence precedence, 
        Associativity Associativity = Associativity::left);
    Callable* create_builtin_unary_operator(const std::string& name, const Type& type, 
        UnaryFixity fixity = UnaryFixity::prefix);

    // container for top-level statements
    Callable* root_;

    std::unique_ptr<Scope> globals_ = std::make_unique<Scope>(this);
    std::unique_ptr<Scope> builtins_scope_ = std::make_unique<Scope>(this);

    bool is_valid = true;
    std::unique_ptr<TypeRegistry> types_ = std::make_unique<TypeRegistry>();

    // layer1 fills these with pointers to expressions that need work so that layer 2 doesn't
    // need to walk the tree to find them
    std::vector<Expression*> unresolved_identifiers_ = {};
    std::vector<Expression*> unresolved_type_identifiers_ = {};
    std::vector<Expression*> unparsed_termed_expressions_ = {};

private:
    friend Scope; // scope is allowed to call create_expression directly to create call expressions
    Expression* create_expression(ExpressionType expression_type, 
        ExpressionValue value, const Type& type, SourceLocation location);

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
void AST::walk_expression(T visitor, Expression* expression) {
    visitor.visit_expression(expression);

    switch (expression->expression_type) {
        case ExpressionType::call:
            // can't visit the call target cause would get into infinite loops
            // !!! TODO: visit lambdas somehow
            for (Expression* arg: std::get<1>(expression->call_value()))
                walk_expression(visitor, arg);
            return;

        case ExpressionType::termed_expression:
            for (Expression* sub_expression: expression->terms())
                walk_expression(visitor, sub_expression);
            return;

        case ExpressionType::type_construct:
        case ExpressionType::type_argument:
            assert(false && "not implemented");
            return;
        
        default:
            return;
    }
}

template<AST_Visitor T>
void AST::walk_statement(T visitor, Statement* statement) {
    visitor.visit_statement(statement);

    switch (statement->statement_type) {
        case StatementType::assignment:
        case StatementType::expression_statement:
        case StatementType::return_:
            walk_expression(visitor, get<Expression*>(statement->value));
            return;
        
        case StatementType::block:
            for (Statement* sub_statement: get<Block>(statement->value))
                walk_statement(visitor, sub_statement);
            return;

        case StatementType::let:
            assert(false && "not implemented");

        case StatementType::operator_s:
        default:
            return;
    }
}

template<AST_Visitor T>
void AST::walk_callable(T visitor, Callable* callable) {
    visitor.visit_callable(callable);

    if (Expression* const* expression = get_if<Expression*>(&callable->body)) {
        walk_expression(visitor, *expression);
    } else if (Statement* const* statement = get_if<Statement*>(&callable->body)) {
        walk_statement(visitor, *statement);
    }
}

template<AST_Visitor T>
void AST::visit_nodes(T visitor) {
    for (auto [_1, callable]: globals_->identifiers_) {
        walk_callable(visitor, callable);
    }

    walk_callable(visitor, root_);
}

} // namespace Maps

#endif