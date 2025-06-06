#ifndef __EXPRESSION_HH
#define __EXPRESSION_HH

#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "common/maps_datatypes.h"
#include "common/deferred_bool.hh"

#include "mapsc/source_location.hh"

#include "mapsc/ast/operator.hh"
#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"

#define OPERATOR_EXPRESSION ExpressionType::operator_identifier:\
                       case ExpressionType::binary_operator_reference:\
                       case ExpressionType::prefix_operator_reference:\
                       case ExpressionType::postfix_operator_reference

#define TYPE_EXPRESSION ExpressionType::type_identifier:\
                   case ExpressionType::type_reference:\
                   case ExpressionType::type_operator_identifier:\
                   case ExpressionType::type_operator_reference:\
                   case ExpressionType::type_constructor_reference:\
                   case ExpressionType::type_field_name:\
                   case ExpressionType::type_construct:\
                   case ExpressionType::type_argument

#define ILLEGAL_EXPRESSION ExpressionType::deleted:\
                      case ExpressionType::compiler_error:\
                      case ExpressionType::user_error

#define NON_CASTABLE_EXPRESSION ExpressionType::minus_sign:\
                           case OPERATOR_EXPRESSION:\
                           case TYPE_EXPRESSION:\
                           case ILLEGAL_EXPRESSION

namespace Maps {

class CompilationState;
class AST_Store;
class Operator;
struct Expression;

// ----- EXPRESSIONS -----

// NOTE: references and calls are created by scopes, rest are created by AST
// See: 'docs/internals/ast\ nodes' for description of what these mean 
enum class ExpressionType {
    known_value,
    
    identifier,                 // value: string
    operator_identifier,
    type_operator_identifier,

    type_identifier,            // value: string
    type_construct,             // value: type_identifier | (type_constructor_identifier, [type_parameter])
    type_argument,              // value: (type_construct, optional<string>)

    minus_sign,                 // minus sign is special, value: std::monostate
    reference,                  // value: Callable*
    known_value_reference,
    binary_operator_reference,
    prefix_operator_reference,
    postfix_operator_reference,
    type_reference,
    type_operator_reference,
    type_constructor_reference,
    type_field_name,

    layer2_expression,          // value: std::vector<Expression*>
    
    user_error,                 // value: std::string
    compiler_error,
    
    call,                       // value: 
    partial_call,
    partial_binop_call_left,
    partial_binop_call_right,
    partial_binop_call_both,
    partially_applied_minus,
    missing_arg,

    // lambda,
    ternary_expression,

    deleted,                    // value: std::monostate
};

using CallExpressionValue = std::tuple<const DefinitionHeader*, std::vector<Expression*>>;

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
    Scope* context;

    DeferredBool is_type_declaration = DeferredBool::maybe_;

    bool operator==(const TermedExpressionValue&) const = default;
};

// struct LambdaExpressionValue {
//     static LambdaExpressionValue const_value(AST_Store& ast_store, Expression* value, 
//         const std::vector<const Type*>& param_types, const SourceLocation& location);

//     ParameterList parameters;
//     std::optional<Scope*> scope;
//     ReferableNode* definition;

//     bool operator==(const LambdaExpressionValue& other) const {
//         return this == &other;
//     };
// };

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
    const DefinitionHeader*,
    const Type*,
    TermedExpressionValue,
    CallExpressionValue,
    // LambdaExpressionValue,
    TernaryExpressionValue,
    TypeArgument,
    TypeConstruct
>;

std::string log_representation(const ExpressionValue& value);

struct Expression {
    // ----- STATIC METHODS -----

    // static Expression* lambda(CompilationState& state, const LambdaExpressionValue& value, 
    //     const Type* return_type, bool is_pure, const SourceLocation& location);
    // static Expression* lambda(CompilationState& state, const LambdaExpressionValue& value, 
    //     bool is_pure, const SourceLocation& location);

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

    // ----- GETTERS etc. -----
    std::vector<Expression*>& terms();
    const std::vector<Expression*>& terms() const;
    Scope* termed_context() const;

    CallExpressionValue& call_value();
    const CallExpressionValue& call_value() const;
    const DefinitionHeader* reference_value() const;
    const Type* type_reference_value() const;
    const Operator* operator_reference_value() const;
    std::optional<KnownValue> known_value_value() const;
    Expression* partially_applied_minus_value() const;

    // LambdaExpressionValue& lambda_value();
    // const LambdaExpressionValue& lambda_value() const;
    
    TernaryExpressionValue& ternary_value();
    const TernaryExpressionValue& ternary_value() const;

    void mark_not_type_declaration();
    DeferredBool is_type_declaration();

    std::string_view string_value() const;
    LogStream::InnerStream& log_self_to(LogStream::InnerStream& ostream) const;
    
    std::optional<Expression*> cast_to(CompilationState& state, const Type* type, 
        const SourceLocation& type_declaration_location);
    std::optional<Expression*> cast_to(CompilationState& state, const Type* type);
    std::optional<Expression*> wrap_in_runtime_cast(CompilationState& state, const Type* type, 
        const SourceLocation& type_declaration_location);

    std::string_view expression_type_string_view() const;
    std::string expression_type_string() const { 
        return std::string{expression_type_string_view()}; }

    bool operator==(const Expression& other) const = default;

    // ----- PUBLIC FIELDS -----
    ExpressionType expression_type; 
    
    ExpressionValue value;
    const Type* type = &Unknown; // this is the "de facto"-one
    std::optional<const Type*> declared_type = std::nullopt;
    SourceLocation location;
};

} // namespace Maps

#endif