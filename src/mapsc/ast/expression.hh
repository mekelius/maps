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

    known_value,
    
    identifier,                // value: string
    operator_identifier,
    type_operator_identifier,

    type_identifier,           // value: string
    type_construct, // value: type_identifier | (type_constructor_identifier, [type_parameter])
    type_argument,             // value: (type_construct, optional<string>)

    minus_sign,                // minus sign is special, value: std::monostate
    reference,                 // value: Callable*
    known_value_reference,
    binary_operator_reference,
    prefix_operator_reference,
    postfix_operator_reference,
    type_reference,
    type_operator_reference,
    type_constructor_reference,
    type_field_name,

    termed_expression,      // value: std::vector<Expression*>
    
    user_error,           // value: std::string
    compiler_error,
    
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
};

using ParameterList = std::vector<Definition*>;

struct LambdaExpressionValue {
    ParameterList parameters;
    RT_Scope* scope;
    DefinitionBody body;

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

using KnownValue = std::variant<maps_Int, maps_Float, bool, std::string, maps_MutString>;

using ExpressionValue = std::variant<
    std::monostate,
    maps_Int,
    maps_Float,
    maps_MutString,
    bool,
    std::string,
    Expression*,
    Definition*,                       // for references to operators and functions
    const Type*,                       // for type expressions
    TermedExpressionValue,
    CallExpressionValue,
    LambdaExpressionValue,
    TernaryExpressionValue,
    TypeArgument,
    TypeConstruct
>;

struct Expression {
    // ----- STATIC METHODS -----
    static Expression* string_literal(AST_Store& store, 
        const std::string& value, const SourceLocation& location);
    static Expression* numeric_literal(AST_Store& store, 
        const std::string& value, const SourceLocation& location);
    static Expression* known_value(AST_Store& store, KnownValue value,
        const SourceLocation& location);
    static std::optional<Expression*> known_value(AST_Store& store, KnownValue value, const Type* type,
        const SourceLocation& location);

    static Expression* identifier(AST_Store& store, RT_Scope* scope, 
        const std::string& value, const SourceLocation& location);
    static Expression* operator_identifier(AST_Store& store, RT_Scope* scope, 
        const std::string& value, const SourceLocation& location);
    static Expression* type_identifier(AST_Store& store, 
        const std::string& value, const SourceLocation& location);
    static Expression* type_operator_identifier(AST_Store& store, 
        const std::string& value, const SourceLocation& location);

    static Expression* termed(AST_Store& store, 
        std::vector<Expression*>&& terms, const SourceLocation& location);

    static Expression* reference(AST_Store& store, 
        Definition* callee, const SourceLocation& location);
    static Expression* type_reference(AST_Store& store, 
        const Type* type, const SourceLocation& location);
    static Expression operator_reference(
        Definition* callee, const SourceLocation& location);
    static Expression* operator_reference(AST_Store& store, 
        Definition* callee, const SourceLocation& location);

    static std::optional<Expression*> reference(AST_Store& store, const Scope& scope, 
        const std::string& name, const SourceLocation& location);
    static std::optional<Expression*> type_operator_reference(AST_Store& store, 
        const std::string& name, const Type* type, const SourceLocation& location);

    static std::optional<Expression*> call(CompilationState& state, 
        Definition* callee, std::vector<Expression*>&& args, const SourceLocation& location);

    static std::optional<Expression*> partial_binop_call(CompilationState& state, 
        Definition* callee, Expression* lhs, Expression* rhs, const SourceLocation& location);

    // not implemented
    static std::optional<Expression*> partial_binop_call_both(CompilationState& state,
        Definition* lhs, Expression* lambda, Definition* rhs, const SourceLocation& location);
    
    static Expression* partially_applied_minus(AST_Store& store, 
        Expression* rhs, const SourceLocation& location);

    static Expression* lambda(CompilationState& state, const LambdaExpressionValue& value, 
        const Type* return_type, bool is_pure, const SourceLocation& location);
    static Expression* lambda(CompilationState& state, const LambdaExpressionValue& value, 
        bool is_pure, const SourceLocation& location);

    static Expression* valueless(AST_Store& store, 
        ExpressionType expression_type, const SourceLocation& location);
    static Expression* missing_argument(AST_Store& store, 
        const Type* type, const SourceLocation& location);
    static Expression* minus_sign(AST_Store& store, const SourceLocation& location);

    static Expression* user_error(AST_Store& store, const SourceLocation& location);
    static Expression* compiler_error(AST_Store& store, const SourceLocation& location);

    static Expression builtin(const ExpressionValue& value, const Type& type) {
        return Expression{ExpressionType::known_value, value, &type, BUILTIN_SOURCE_LOCATION};
    }

    static std::string value_to_string(const ExpressionValue& value);
    static std::string value_to_string(const KnownValue& value);

    // ----- CONSTRUCTORS -----
    Expression(ExpressionType expression_type, const SourceLocation& location)
    :expression_type(expression_type), value(std::monostate{}), location(location) {}

    Expression(ExpressionType expression_type, ExpressionValue value, const SourceLocation& location)
    :expression_type(expression_type), value(value), location(location) {}

    Expression(ExpressionType expression_type, ExpressionValue value, const Type* type, 
        const SourceLocation& location)
    :expression_type(expression_type), value(value), type(type), location(location) {}

    Expression(ExpressionType expression_type, ExpressionValue value, const Type* type, 
        const Type* declared_type, const SourceLocation& location)
    :expression_type(expression_type), value(value), type(type), declared_type(declared_type), 
     location(location) {}

    // ----- CONVERSIONS -----
    // expect to be a partially applied minus
    void convert_to_partial_binop_minus_call_left(AST_Store& store);
    void convert_to_unary_minus_call();
    
    // For example partial binop call, currently a no-op
    void convert_to_partial_call();

    void convert_to_reference(Definition* callee);
    void convert_to_operator_reference(Definition* callee);

    // ----- GETTERS etc. -----
    std::vector<Expression*>& terms();
    const std::vector<Expression*>& terms() const;

    CallExpressionValue& call_value();
    Definition* reference_value() const;
    const Type* type_reference_value() const;
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
    
    std::optional<Expression*> cast_to(CompilationState& state, const Type* type, 
        const SourceLocation& type_declaration_location);
    std::optional<Expression*> cast_to(CompilationState& state, const Type* type);
    std::optional<Expression*> wrap_in_runtime_cast(CompilationState& state, const Type* type, 
        const SourceLocation& type_declaration_location);

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