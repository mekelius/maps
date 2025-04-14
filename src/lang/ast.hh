#ifndef __AST_HH
#define __AST_HH

#include <vector>
#include <unordered_map>
#include <tuple>
#include <string>
#include <memory>
#include <string_view>
#include <optional>
#include <cassert>
#include <variant>

#include "pragmas.hh"
#include "types.hh"
#include "../logging.hh"

namespace AST {

class AST;
struct Expression;
struct Statement;

using CallableBody = std::variant<std::monostate, Expression*, Statement*>;

// ----- EXPRESSIONS -----

enum class ExpressionType {
    string_literal = 0,     // literal: Value value
    numeric_literal,
    identifier,
    call,                   // call: Callee identifier, [Expression] args
    deferred_call,          // call where the callee is an expression
    builtin_function,
    builtin_operator,
    termed_expression,      // basically something that layer1 can't handle
    tie,                    // lack of whitespace between an operator and another term
    unresolved_identifier,  // something to be hoisted
    unresolved_operator,
    syntax_error,           // reached something that shouldn't have been such as trying to parse eof as expression
    not_implemented,
    deleted,
};

using CallExpressionValue = std::tuple<std::string, std::vector<Expression*>>;
using CallExpressionValueDeferred = std::tuple<Expression*, std::vector<Expression*>>;
using TermedExpressionValue = std::vector<Expression*>;
using TermedExpressionValue = std::vector<Expression*>;

using ExpressionValue = std::variant<
    std::monostate,
    std::string,
    CallExpressionValue,
    CallExpressionValueDeferred,
    TermedExpressionValue
>;

struct Expression {
    // TODO: move initializing expression values from AST::create_expression
    Expression(ExpressionType expr_type, SourceLocation location, const Type& type): 
    expression_type(expr_type), location(location), type(type) {};
    
    ExpressionType expression_type;
    SourceLocation location;
    Type type = Hole;
    ExpressionValue value;

    TermedExpressionValue& terms() {
        return std::get<TermedExpressionValue>(value);
    }
    CallExpressionValue& call() {
        return std::get<CallExpressionValue>(value);
    }
    std::string& string_value() {
        return std::get<std::string>(value);
    }
};

// ----- STATEMENTS -----
struct Let{
    std::string identifier; 
    CallableBody body;
};

struct Assignment{
    std::string identifier; 
    CallableBody body;
};

using Block = std::vector<Statement*>;

enum class StatementType {
    broken,                 // parsing failed
    illegal,                // well formed but illegal statements
    empty,
    expression_statement,   // statement consisting of a single expression
    block,
    let,
    assignment,
    return_,
    // if,
    // else,
    // for,
    // for_id,
    // do_while,
    // do_for,
    // while/until,
    // switch,
};

using StatementValue = std::variant<
    std::monostate,
    std::string,
    Expression*,

    Let,
    Assignment,
    Block
>;

struct Statement {
    Statement(StatementType statement_type, SourceLocation location);
    
    StatementType statement_type;
    SourceLocation location;
    StatementValue value;
};

/**
 * Callables represent either expressions or statements, with the unique ability
 * to hold types for statements. A bit convoluted but we'll see.
 */
class Callable {
  public:
    Callable(CallableBody body, SourceLocation location);

    SourceLocation location;
    CallableBody body;

    // since statements don't store types, we'll have to store them here
    // if the body is an expression, the type will just mirror it's type
    Type get_type() const;
    void set_type(Type& type);

  private:
    std::optional<Type> type_;
};

/**
 * Scopes contain names bound to callables
 * Note that it is the AST that owns the callables, but they can be created through the scope
 */
class Scope {
  public:
    Scope(AST* ast): ast_(ast) {};

    std::optional<Callable*> create_identifier(const std::string& name, CallableBody body, 
        SourceLocation location);
    std::optional<Callable*> create_identifier(const std::string& name, SourceLocation location);
   
    bool identifier_exists(const std::string& name) const;
    std::optional<Callable*> get_identifier(const std::string& name) const;
  
    std::vector<std::string> identifiers_in_order = {};

  private:
    std::unordered_map<std::string, Callable*> identifiers_;
    AST* ast_;
};

// TODO: expand mind enough for contexts
// context where statements can exist
// class Context {
//   public:
//     enum Type {
//         local,
//         global,
//         global_eval, // global context where evaluation is allowed 
//     };

//     Context::Type context_type = local;
//     std::optional<Scope> scope = std::nullopt;
// };

class AST {
  public:
    AST();

    Expression* create_expression(ExpressionType expression_type, SourceLocation location, 
        const Type& type = Void);
    Statement* create_statement(StatementType statement_type, SourceLocation location);
    Callable* create_callable(CallableBody body, SourceLocation location);
    Callable* create_callable(SourceLocation location);

    // appends a statement to root_
    void append_top_level_statement(Statement* statement);

    // container for top-level statements
    std::vector<Statement*> root_ = {};   
    Scope globals_;
    Scope builtins_ = { this };
    Scope builtin_operators_ = { this };

    bool is_valid = true;

    // layer1 fills these with pointers to expressions that need work so that layer 2 doesn't 
    // need to walk the tree to find them
    std::vector<Expression*> unresolved_identifiers_and_operators = {};
    std::vector<Expression*> unparsed_termed_expressions = {};
    
  private:
    // currently these guys, once created, stay in memory forever
    // we could create a way to sweep them by having some sort of "alive"-flag
    // or maybe "DeletedStatement" statement type
    // probably won't need that until we do the interpreter
    std::vector<std::unique_ptr<Statement>> statements_ = {};
    std::vector<std::unique_ptr<Expression>> expressions_ = {};
    std::vector<std::unique_ptr<Callable>> callables_ = {};
};

} // namespace AST

#endif