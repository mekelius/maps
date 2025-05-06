#ifndef __AST_NODE_HH
#define __AST_NODE_HH

#include <string>
#include <tuple>

#include "mapsc/source.hh"

#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/callable.hh"
#include "mapsc/ast/operator.hh"

namespace Maps {

// ----- BUILTINS -----

struct Builtin {
    std::string name;
    const Type* type;
};

class Callable;
struct Expression;
struct Statement;

// ----- EXPRESSIONS -----

// NOTE: references and calls are created by scopes, rest are created by AST
// See: 'docs/internals/ast\ nodes' for description of what these mean 
enum struct ExpressionType {
    string_literal = 0,             // value: string
    numeric_literal,

    value,
    
    identifier,                     // value: string
    operator_identifier,
    type_operator_identifier,

    type_identifier,                // value: string
    type_construct,                 // value: type_identifier | (type_constructor_identifier, [type_parameter])
    type_argument,                  // value: (type_construct, optional<string>)

    reference,                      // value: Callable*
    operator_reference,
    type_reference,
    type_operator_reference,
    type_constructor_reference,
    type_field_name,

    termed_expression,      // value: std::vector<Expression*>
    
    syntax_error,           // value: std::string
    not_implemented,
    
    call,                   // value: 
    missing_arg,

    deleted,                // value: std::monostate
};

using CallExpressionValue = std::tuple<Callable*, std::vector<Expression*>>;

using TypeArgument = std::tuple<
    Expression*,                // type construct or type identifier
    std::optional<std::string>  // optional name for a named arg
>;

using TypeConstruct = std::tuple<
    Expression*,                // type or type_constructor identifier
    std::vector<Expression*>    // type_arguments
>;

struct TermedExpressionValue {
    std::vector<Expression*> terms;
    DeferredBool is_type_declaration = DeferredBool::maybe_;

    bool operator==(const TermedExpressionValue&) const = default;

    std::string to_string() const;
};

using ExpressionValue = std::variant<
    std::monostate,
    long,
    double,
    bool,
    std::string,
    Callable*,                       // for references to operators and functions
    const Type*,                     // for type expressions
    TermedExpressionValue,
    CallExpressionValue,
    TypeArgument,
    TypeConstruct
>;

struct Expression {
public:
    ExpressionType expression_type; 
    SourceLocation location;

    ExpressionValue value;

    const Type* type = &Hole; // this is the "de facto"-one
    std::optional<const Type*> declared_type = std::nullopt;
    
    // ----- METHODS -----
    bool convert_to_native_types();

    // ----- GETTERS etc. -----
    std::vector<Expression*>& terms();
    CallExpressionValue& call_value();
    Callable* reference_value() const;

    bool is_partial_call() const;
    bool is_reduced_value() const;

    void mark_not_type_declaration();
    DeferredBool is_type_declaration();

    const std::string& string_value() const;
    std::string log_message_string() const;
    
    DeferredBool has_native_representation();
    bool is_literal() const;
    bool is_illegal() const;
    bool is_reference() const;
    bool is_identifier() const;
    bool is_ok_in_layer2() const;
    bool is_ok_in_codegen() const;
    bool is_castable_expression() const;
    bool is_allowed_in_type_declaration() const;

    bool operator==(const Expression& other) const = default;
};

// ----- STATEMENTS -----
struct Let {
    std::string identifier; 
    CallableBody body;

    bool operator==(const Let& other) const = default;
};

struct OperatorStatementValue {
    std::string op;
    unsigned int arity;
    CallableBody body;
    // include the specifiers
    bool operator==(const OperatorStatementValue& other) const = default;

};

struct Assignment {
    std::string identifier; 
    CallableBody body;

    bool operator==(const Assignment& other) const = default;
};

using Block = std::vector<Statement*>;

enum class StatementType {
    deleted,
    broken,                 // parsing failed
    illegal,                // well formed but illegal statements
    empty,
    expression_statement,   // statement consisting of a single expression
    block,
    let,
    operator_definition,
    assignment,
    return_,
    // if,
    // else,
    // for,
    // for_in,
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
    OperatorStatementValue,
    Assignment,
    Block
>;

struct Statement {
    Statement(StatementType statement_type, SourceLocation location);
    
    StatementType statement_type;
    SourceLocation location;
    StatementValue value;

    std::string log_message_string() const;

    bool is_illegal_as_single_statement_block() const;

    bool operator==(const Statement& other) const = default;
};

} // namespace Maps

#endif