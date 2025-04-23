#ifndef __AST_HH
#define __AST_HH

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include "types.hh"
#include "ast_node.hh"
#include "scope.hh"

namespace AST {

class AST {
  public:
    AST();
    void set_root(CallableBody root);
    void declare_invalid() { is_valid = false; };

    // ----- CREATING (AND DELETING) EXPRESSIONS -----
    Expression* create_string_literal(const std::string& value, SourceLocation location);
    Expression* create_numeric_literal(const std::string& value, SourceLocation location);
    
    // These automatically add the identifier into unresolved list as a convenience
    Expression* create_identifier_expression(const std::string& value, SourceLocation location);
    Expression* create_operator_expression(const std::string& value, SourceLocation location);

    Expression* create_termed_expression(std::vector<Expression*>&& terms, SourceLocation location);

    std::optional<Expression*> create_operator_ref(const std::string& name, SourceLocation location);
    Expression* create_operator_ref(Callable* callable, SourceLocation location);

    // valueless expression types are tie, empty, syntax_error and not_implemented
    Expression* create_valueless_expression(ExpressionType expression_type, SourceLocation location);
    Expression* create_missing_argument(const Type& type, SourceLocation location);
    
    void delete_expression(Expression* expression);


    //  ----- CREATING OTHER THINGS -----
    Statement* create_statement(StatementType statement_type, SourceLocation location);
    
    // automatically creates an identifier and a global callable for the builtin
    Callable* create_builtin(BuiltinType builtin_type, const std::string& name, const Type& type);

    // container for top-level statements
    Callable* root_;
    Callable* entry_point_;

    std::unique_ptr<Scope> globals_ = std::make_unique<Scope>(this);
    std::unique_ptr<Scope> builtin_functions_ = std::make_unique<Scope>(this);
    std::unique_ptr<Scope> builtin_operators_ = std::make_unique<Scope>(this);

    bool is_valid = true;

    // layer1 fills these with pointers to expressions that need work so that layer 2 doesn't
    // need to walk the tree to find them
    std::vector<Expression*> unresolved_identifiers_and_operators = {};
    std::vector<Expression*> unparsed_termed_expressions = {};
    
  private:
    friend Scope; // scope is allowed to call create_expression directly to create call expressions
    Expression* create_expression(ExpressionType expression_type, 
        ExpressionValue value, const Type& type, SourceLocation location);

    Callable* create_callable(CallableBody body, const std::string& name, std::optional<SourceLocation> location = std::nullopt);
    Callable* create_callable(const std::string& name, SourceLocation location);

    // currently these guys, once created, stay in memory forever
    // we could create a way to sweep them by having some sort of "alive"-flag
    // or maybe "DeletedStatement" statement type
    // probably won't need that until we do the interpreter
    // TODO: move from vector of unique_ptrs to unique_ptr of vectors
    std::vector<std::unique_ptr<Statement>> statements_ = {};
    std::vector<std::unique_ptr<Expression>> expressions_ = {};
    std::vector<std::unique_ptr<Builtin>> builtins_ = {};
    std::vector<std::unique_ptr<Callable>> callables_ = {};
};

} // namespace AST

#endif