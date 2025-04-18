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
struct Builtin;
class Callable;

using CallableBody = std::variant<std::monostate, Expression*, Statement*, Builtin*>;

// ----- BUILTINS -----

enum class BuiltinType {
    builtin_function,
    builtin_operator,
};

struct Builtin {
    BuiltinType builtin_type;
    std::string name;
    Type type;
};

// ----- EXPRESSIONS -----

// NOTE: references and calls are created by scopes, rest are created by AST
enum class ExpressionType {
// layer1
    string_literal = 0,     // value: string
    numeric_literal,
    
    identifier,             // value: string
    operator_e,
    
    reference,              // value: Callable*
    operator_ref,           

    termed_expression,      // value: std::vector<Expression*>

    tie,                    // value: std::monostate
    empty,
    
    syntax_error,           // value: std::string
    not_implemented,
    
// layer2
    call,                   // value: 
    // deferred_call,          // value: std::tuple<Expression*, std::vector<Expression*>>
    missing_arg,

    // TODO: replace these with calls to simplify

    deleted,                // value: std::monostate

// misc
};

using CallExpressionValue = std::tuple<Callable*, std::vector<Expression*>>;
// using CallExpressionValueDeferred = std::tuple<Expression*, std::vector<Expression*>>;
using TermedExpressionValue = std::vector<Expression*>;

using ExpressionValue = std::variant<
    std::monostate,
    std::string,
    CallExpressionValue,
    // CallExpressionValueDeferred,
    TermedExpressionValue,
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
    CallExpressionValue& call_value() {
        return std::get<CallExpressionValue>(value);
    }
    Callable* reference_value() const {
        return std::get<Callable*>(value);
    }
    bool is_partial_call() const;

    const std::string& string_value() const;

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
struct Let {
    std::string identifier; 
    CallableBody body;
};

struct Operator {
    std::string op;
    unsigned int arity;
    CallableBody body;
    // include the specifiers
};

struct Assignment {
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
    operator_s,
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
    Operator,
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
 * The source location is needed on every callable that is not a built-in
 * 
 * NOTE: if it's anonymous, it needs a source location
 */
class Callable {
  public:
    Callable(CallableBody body, const std::string& name, std::optional<SourceLocation> location = std::nullopt);
    Callable(CallableBody body, std::optional<SourceLocation> location); // create anonymous callable

    CallableBody body;
    std::string name;
    std::optional<SourceLocation> location;

    // since statements don't store types, we'll have to store them here
    // if the body is an expression, the type will just mirror it's type
    Type get_type() const;
    void set_type(const Type& type);

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

    bool identifier_exists(const std::string& name) const;
    std::optional<Callable*> get_identifier(const std::string& name) const;

    std::optional<Callable*> create_callable(const std::string& name, CallableBody body, 
        std::optional<SourceLocation> location = std::nullopt);
    std::optional<Callable*> create_callable(const std::string& name, SourceLocation location);

    std::optional<Callable*> create_binary_operator(const std::string& name, CallableBody body, 
        unsigned int precedence, Associativity associativity, SourceLocation location);

    std::optional<Callable*> create_unary_operator(const std::string& name, CallableBody body,
        Fixity fixity, SourceLocation location);

    std::optional<Expression*> create_reference_expression(const std::string& name, SourceLocation location);
    Expression* create_reference_expression(Callable* callable, SourceLocation location);

    std::optional<Expression*> create_call_expression(
        const std::string& callee_name, std::vector<Expression*> args, SourceLocation location /*, expected return type?*/);
    Expression* create_call_expression(Callable* callee, std::vector<Expression*> args, 
        SourceLocation location /*, expected return type?*/);

    std::vector<std::string> identifiers_in_order_ = {};
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

    // ----- CREATING EXPRESSIONS -----
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
    
    // appends a statement to root_
    void append_top_level_statement(Statement* statement);
    
    // automatically creates an identifier and a global callable for the builtin
    Callable* create_builtin(BuiltinType builtin_type, const std::string& name, const Type& type);

    void declare_invalid() { is_valid = false; };

    // container for top-level statements
    std::vector<Statement*> root_ = {};   

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