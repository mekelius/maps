#ifndef __AST_NODE_HH
#define __AST_NODE_HH

#include <string>

#include "../source.hh"
#include "type_defs.hh"

namespace Maps {

// ----- BUILTINS -----

enum class BuiltinType {
    builtin_function,
    builtin_operator,
};

struct Builtin {
    BuiltinType builtin_type;
    std::string name;
    const Type* type;
};

struct Expression;
struct Statement;

using CallableBody = std::variant<std::monostate, Expression*, Statement*, Builtin*>;

/**
 * Callables represent either expressions or statements, with the unique ability
 * to hold types for statements. A bit convoluted but we'll see.
 * The source location is needed on every callable that is not a built-in
 * 
 * NOTE: if it's anonymous, it needs a source location
 */
class Callable {
public:
    Callable(CallableBody body, const std::string& name,
        std::optional<SourceLocation> location = std::nullopt);
    Callable(CallableBody body, std::optional<SourceLocation> location); // create anonymous callable

    Callable(const Callable& other) = default;
    Callable& operator=(const Callable& other) = default;
    virtual ~Callable() = default;

    CallableBody body;
    std::string name;
    std::optional<SourceLocation> location;

    // since statements don't store types, we'll have to store them here
    // if the body is an expression, the type will just mirror it's type
    const Type* get_type() const;
    void set_type(const Type& type);

private:
    std::optional<const Type*> type_;
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
        expression_type(expr_type), location(location), type(&type) {};
    
    ExpressionType expression_type;
    SourceLocation location;
    const Type* type;
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
    bool is_reduced_value() const;

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

} // namespace Maps

#endif