#ifndef __EXPRESSION_HH
#define __EXPRESSION_HH

#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "common/maps_datatypes.h"

#include "mapsc/source.hh"

#include "mapsc/ast/operator.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"

namespace Maps {

class CompilationState;
class AST_Store;
class Definition;
class Operator;
struct Expression;

// ----- EXPRESSIONS -----

// NOTE: references and calls are created by scopes, rest are created by AST
// See: 'docs/internals/ast\ nodes' for description of what these mean 
enum class ExpressionType {
    string_literal = 0,        // value: string
    numeric_literal,

    value,
    
    identifier,                // value: string
    operator_identifier,
    type_operator_identifier,

    type_identifier,           // value: string
    type_construct, // value: type_identifier | (type_constructor_identifier, [type_parameter])
    type_argument,             // value: (type_construct, optional<string>)

    minus_sign,                // minus sign is special, value: std::monostate
    reference,                 // value: Callable*
    binary_operator_reference,
    prefix_operator_reference,
    postfix_operator_reference,
    type_reference,
    type_operator_reference,
    type_constructor_reference,
    type_field_name,

    termed_expression,      // value: std::vector<Expression*>
    
    syntax_error,           // value: std::string
    compiler_error,
    not_implemented,
    
    call,                   // value: 
    partial_call,
    partial_binop_call_left,
    partial_binop_call_right,
    partial_binop_call_both,
    partially_applied_minus,
    missing_arg,

    lambda,
    ternary_expression,

    deleted,                // value: std::monostate
};

using CallExpressionValue = std::tuple<Definition*, std::vector<Expression*>>;

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

struct LambdaExpressionValue {
    Expression* binding_type_declaration;
    DefinitionBody body;
    std::optional<RT_Scope> scope = std::nullopt;

    bool operator==(const LambdaExpressionValue& other) const {
        return this == &other;
    };
};

struct TernaryExpressionValue {
    Expression* condition;
    Expression* success;
    Expression* failure;

    bool operator==(const TernaryExpressionValue&) const = default;
};

using ExpressionValue = std::variant<
    std::monostate,
    maps_Int,
    maps_Float,
    bool,
    std::string,
    Expression*,
    Definition*,                       // for references to operators and functions
    const Type*,                     // for type expressions
    TermedExpressionValue,
    CallExpressionValue,
    LambdaExpressionValue,
    TernaryExpressionValue,
    TypeArgument,
    TypeConstruct
>;

struct Expression {
    // ----- STATIC METHODS -----
    static Expression* string_literal(
        AST_Store& store, const std::string& value, SourceLocation location);
    static Expression* numeric_literal(
        AST_Store& store, const std::string& value, SourceLocation location);

    static Expression* identifier(
        CompilationState& state, const std::string& value, SourceLocation location);
    static Expression* type_identifier(
        CompilationState& state, const std::string& value, SourceLocation location);
    static Expression* operator_identifier(
        CompilationState& state, const std::string& value, SourceLocation location);
    static Expression* type_operator_identifier(
        CompilationState& state, const std::string& value, SourceLocation location);

    static Expression* termed(
        AST_Store& store, std::vector<Expression*>&& terms, SourceLocation location);

    static Expression* reference(
        AST_Store& store, Definition* definition, SourceLocation location);
    static Expression* type_reference(
        AST_Store& store, const Type* type, SourceLocation location);
    static Expression operator_reference(
        Definition* definition, SourceLocation location);
    static Expression* operator_reference(
        AST_Store& store, Definition* definition, SourceLocation location);

    static std::optional<Expression*> reference(
        AST_Store& store, const Scope& scope, const std::string& name, SourceLocation location);
    static std::optional<Expression*> type_operator_reference(
        AST_Store& store, const std::string& name, const Type* type, SourceLocation location);

    static std::optional<Expression*> call(
        CompilationState& state, Definition* definition, std::vector<Expression*>&& args, 
        SourceLocation location);

    static std::optional<Expression*> partial_binop_call(CompilationState& state, 
        Definition* definition, Expression* lhs, Expression* rhs, SourceLocation location);

    // not implemented
    static std::optional<Expression*> partial_binop_call_both(CompilationState& state,
        Definition* lhs, Expression* lambda, Definition* rhs, SourceLocation location);
    
    static Expression* partially_applied_minus(AST_Store& store, 
        Expression* rhs, SourceLocation location);

    static Expression* lambda(AST_Store& store, Expression* binding_type_declaration, 
        DefinitionBody body, SourceLocation location);

    static Expression* valueless(
        AST_Store& store, ExpressionType expression_type, SourceLocation location);
    static Expression* missing_argument(
        AST_Store& store, const Type* type, SourceLocation location);
    static Expression* minus_sign(
        AST_Store& store, SourceLocation location);

    static Expression* syntax_error(AST_Store& store, SourceLocation location);

    static Expression builtin(const ExpressionValue& value, const Type& type) {
        return Expression{ExpressionType::value, value, &type, BUILTIN_SOURCE_LOCATION};
    }

    // ----- CONSTRUCTORS -----
    Expression(ExpressionType expression_type, SourceLocation location)
    :expression_type(expression_type), value(std::monostate{}), location(location) {}

    Expression(ExpressionType expression_type, ExpressionValue value, SourceLocation location)
    :expression_type(expression_type), value(value), location(location) {}

    Expression(ExpressionType expression_type, ExpressionValue value, const Type* type, 
        SourceLocation location)
    :expression_type(expression_type), value(value), type(type), location(location) {}

    Expression(ExpressionType expression_type, ExpressionValue value, const Type* type, 
        const Type* declared_type, SourceLocation location)
    :expression_type(expression_type), value(value), type(type), declared_type(declared_type), 
     location(location) {}

    // ----- CONVERSIONS -----
    // expect to be a partially applied minus
    void convert_to_partial_binop_minus_call_left(AST_Store& store);
    void convert_to_unary_minus_call();
    
    // For example partial binop call, currently a no-op
    void convert_to_partial_call();

    void convert_to_reference(Definition* definition);
    void convert_to_operator_reference(Definition* definition);

    // ----- GETTERS etc. -----
    std::vector<Expression*>& terms();
    const std::vector<Expression*>& terms() const;

    CallExpressionValue& call_value();
    Definition* reference_value() const;
    Definition* operator_reference_value() const;

    LambdaExpressionValue& lambda_value();
    const LambdaExpressionValue& lambda_value() const;
    
    TernaryExpressionValue& ternary_value();
    const TernaryExpressionValue& ternary_value() const;

    bool is_partial_call() const;
    bool is_reduced_value() const;

    void mark_not_type_declaration();
    DeferredBool is_type_declaration();

    std::string string_value() const;
    std::string log_message_string() const;
    
    bool is_literal() const;
    bool is_illegal() const;
    bool is_reference() const;
    bool is_identifier() const;
    bool is_ok_in_layer2() const;
    bool is_ok_in_codegen() const;
    bool is_castable_expression() const;
    bool is_allowed_in_type_declaration() const;
    bool is_constant_value() const;

    bool operator==(const Expression& other) const = default;

    // ----- PUBLIC FIELDS -----
    ExpressionType expression_type; 
    
    ExpressionValue value;
    const Type* type = &Hole; // this is the "de facto"-one
    std::optional<const Type*> declared_type = std::nullopt;
    SourceLocation location;
};

Operator::Precedence get_operator_precedence(const Expression& operator_ref);

} // namespace Maps

#endif