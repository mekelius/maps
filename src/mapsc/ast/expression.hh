#ifndef __EXPRESSION_HH
#define __EXPRESSION_HH

#include <variant>
#include <tuple>
#include <optional>
#include <vector>

#include "common/maps_datatypes.h"
#include "mapsc/source.hh"
#include "operator.hh"
#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"

namespace Maps {

class Scope;
class CompilationState;
class AST_Store;
class Callable;
class Operator;

// ----- EXPRESSIONS -----

// NOTE: references and calls are created by scopes, rest are created by AST
// See: 'docs/internals/ast\ nodes' for description of what these mean 
enum class ExpressionType {
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
    maps_Int,
    maps_Float,
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
    ExpressionType expression_type; 
    SourceLocation location;
    
    ExpressionValue value;
    
    const Type* type = &Hole; // this is the "de facto"-one
    std::optional<const Type*> declared_type = std::nullopt;

    // ----- GETTERS etc. -----
    std::vector<Expression*>& terms();
    CallExpressionValue& call_value();
    Callable* reference_value() const;
    Operator* operator_reference_value() const;

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

// ----- CREATING (AND DELETING) EXPRESSIONS -----
Expression* create_string_literal(AST_Store& store, const std::string& value, SourceLocation location);
Expression* create_numeric_literal(AST_Store& store, const std::string& value, SourceLocation location);

Expression* create_identifier_expression(CompilationState& state, const std::string& value,
    SourceLocation location);
Expression* create_type_identifier_expression(CompilationState& state, const std::string& value, 
    SourceLocation location);
Expression* create_operator_identifier_expression(CompilationState& state, const std::string& value, 
    SourceLocation location);
Expression* create_type_operator_identifier_expression(CompilationState& state, const std::string& value, 
    SourceLocation location);

Expression* create_termed_expression(AST_Store& store, std::vector<Expression*>&& terms, 
    SourceLocation location);

Expression* create_reference_expression(AST_Store& store, Callable* callable, SourceLocation location);
std::optional<Expression*> create_reference_expression(AST_Store& store, const Scope& scope, const std::string& name, SourceLocation location);

[[nodiscard]] std::optional<Expression*> create_type_operator_ref(AST_Store& store, 
    const std::string& name, SourceLocation location, const Type* type);

Expression* create_type_reference(AST_Store& store, const Type* type, SourceLocation location);
[[nodiscard]] std::optional<Expression*> create_operator_ref(AST_Store& store, const std::string& name, 
    SourceLocation location, const Type* type);
Expression* create_operator_ref(AST_Store& store, Callable* callable, SourceLocation location);

// valueless expression types are tie, empty, syntax_error and not_implemented
Expression* create_valueless_expression(AST_Store& store, ExpressionType expression_type, 
    SourceLocation location);
Expression* create_missing_argument(AST_Store& store, SourceLocation location, const Type* type);

std::optional<Expression*> create_call_expression(AST_Store& store, SourceLocation location, Callable* callable, 
    const std::vector<Expression*>& args);

Precedence get_operator_precedence(const Expression& operator_ref);

} // namespace Maps

#endif