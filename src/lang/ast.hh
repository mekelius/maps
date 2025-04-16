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
class Callable;

using CallableBody = std::variant<std::monostate, Expression*, Statement*>;

// ----- EXPRESSIONS -----

enum class ExpressionType {
    string_literal = 0,     // literal: Value value
    numeric_literal,
    identifier,
    operator_ref,

    builtin_function,       // TODO: these shouldn't really be expressions
    builtin_operator,       // TODO: they should be specail types of callables
    
    termed_expression,      // basically something that layer1 can't handle
    tie,                    // lack of whitespace between an operator and another term
    empty,
    
    unresolved_identifier,  // something to be hoisted
    unresolved_operator,
    
    syntax_error,           // reached something that shouldn't have been such as trying to parse eof as expression
    not_implemented,
    deleted,
    
// layer2
    call,                   // call: Callee identifier, [Expression] args
    deferred_call,          // call where the callee is an expression
    binary_operator_apply,
    unary_operator_apply,
};

using CallExpressionValue = std::tuple<std::string, std::vector<Expression*>>;
using CallExpressionValueDeferred = std::tuple<Expression*, std::vector<Expression*>>;
using TermedExpressionValue = std::vector<Expression*>;
using TermedExpressionValue = std::vector<Expression*>;
using BinaryOperatorApplyValue = std::tuple<Callable*, Expression*, Expression*>;
using UnaryOperatorApplyValue = std::tuple<Callable*, Expression*>;

using ExpressionValue = std::variant<
    std::monostate,
    std::string,
    CallExpressionValue,
    CallExpressionValueDeferred,
    TermedExpressionValue,
    BinaryOperatorApplyValue,
    UnaryOperatorApplyValue,
    Callable*                       // for references to operators and functions
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
    BinaryOperatorApplyValue binop_apply_value() {
        return std::get<BinaryOperatorApplyValue>(value);
    }
    UnaryOperatorApplyValue unop_apply_value() {
        return std::get<UnaryOperatorApplyValue>(value);
    }
    Callable* callable_ref() {
        return std::get<Callable*>(value);
    }

    friend bool operator==(const Expression& lhs, const Expression& rhs) {
        return std::tie(
            lhs.expression_type,
            lhs.location,
            lhs.type,
            lhs.value
        ) == std::tie(
            rhs.expression_type,
            rhs.location,
            rhs.type,
            rhs.value
        );
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
    void declare_invalid() { is_valid = false; };

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