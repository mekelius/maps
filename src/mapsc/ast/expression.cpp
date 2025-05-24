#include "expression.hh"

#include <memory>
#include <span>
#include <cassert>
#include <sstream>
#include <variant>
#include <string>

#include "common/std_visit_helper.hh"
#include "mapsc/logging.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/procedures/reverse_parse.hh"

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

using std::optional, std::nullopt, std::to_string;

namespace Maps {

using Log = LogNoContext;

// ----- STATIC METHODS -----

std::string Expression::value_to_string(const KnownValue& value) {
    return std::visit(overloaded{
        [](const std::string& value)->std::string { return value; },
        [](auto value)->std::string { return std::to_string(value); },
    }, value);
}

// workaround to deal with ambiguity
std::string known_value_to_string(const KnownValue& value) {
    return Expression::value_to_string(value);
}

std::string Expression::value_to_string(const ExpressionValue& value) {
    return std::visit(overloaded{
        [](std::monostate)->std::string { return "@Undefined expression value@"; },
        [](Expression*)->std::string { return "@reference@"; },
        [](Definition* target)->std::string { return "@reference to " + target->name_string() + "@"; },                       
        [](const Type* type)->std::string { return "@type: " + type->name_string() + "@"; },
        [](TermedExpressionValue)->std::string { return "@unparsed termed expression@"; },
        [](CallExpressionValue)->std::string { return "@call@"; },
        [](LambdaExpressionValue)->std::string { return "@lambda@"; },
        [](TernaryExpressionValue)->std::string { return "@ternary expression value@"; },
        [](TypeArgument)->std::string { return "@type argument@"; },
        [](TypeConstruct)->std::string { return "@type construct@"; },

        [](auto value)->std::string { return known_value_to_string(value); }
    }, value);
}


// ----- EXPRESSION -----

bool Expression::is_reduced_value() const {
    switch (expression_type) {
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
        case ExpressionType::known_value:
        return true;
        
        case ExpressionType::reference:
        case ExpressionType::call:
        default:
            return false;
    }
}

std::string Expression::string_value() const {
    if (std::holds_alternative<Definition*>(value)) {
        // !!! this will cause crashes when lambdas come in
        return std::get<Definition*>(value)->name_string();
    }
    return std::get<std::string>(value);
}

bool Expression::is_illegal() const {
    switch (expression_type) {
        case ExpressionType::deleted:
        case ExpressionType::user_error:
        case ExpressionType::compiler_error:
            return true;

        default:
            return false;
    }
}

bool Expression::is_ok_in_layer2() const {
    return !(is_identifier() || is_illegal() || expression_type == ExpressionType::type_operator_reference);
}

bool Expression::is_ok_in_codegen() const {
    switch (expression_type) {
        case ExpressionType::reference:
        case ExpressionType::call:
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
            return true;

        default:
            return false;
    }
}

bool Expression::is_castable_expression() const {
    switch (expression_type) {
        case NON_CASTABLE_EXPRESSION:
            return false;

        default:
            return true;
    }
}

bool Expression::is_allowed_in_type_declaration() const {
    switch (expression_type) {
        case ExpressionType::termed_expression:
            return std::get<TermedExpressionValue>(value).is_type_declaration != 
                DeferredBool::false_;

        case ExpressionType::type_argument:
        case ExpressionType::type_construct:
        case ExpressionType::type_operator_identifier:
        case ExpressionType::type_operator_reference:
        case ExpressionType::type_constructor_reference:
        case ExpressionType::type_identifier:
        case ExpressionType::type_reference:
        case ExpressionType::type_field_name:
            return true;

        case ExpressionType::identifier:
            return true;

        default:
            return false;
    }
}

std::string Expression::log_message_string() const {
    switch (expression_type) {
        case ExpressionType::type_construct:
            return "type argument (TODO pretty print)";
            // TOOD:
            // return type_argument_value().to_string();

        case ExpressionType::type_argument:
            return "type argument (TODO pretty print)";
            // TOOD:
            // return type_argument_value().to_string();

        case ExpressionType::termed_expression:
            return Expression::value_to_string(value);

        case ExpressionType::string_literal:
            return "string literal \"" + string_value() + "\"";

        case ExpressionType::numeric_literal:
            return "numeric literal +" + string_value();
    
        case ExpressionType::known_value:
            return "value expression " + Expression::value_to_string(value);
        
        case ExpressionType::identifier:
            return "identifier " + string_value();

        case ExpressionType::operator_identifier:
            return "operator " + string_value();
        case ExpressionType::type_operator_identifier:
            return "type operator " + string_value();
        case ExpressionType::type_identifier:
            return "type identifier " + string_value();

        case ExpressionType::reference:
        case ExpressionType::known_value_reference:
            return "reference to " + reference_value()->name_string();
        case ExpressionType::type_reference:
            return "reference to type " + reference_value()->name_string();
        case ExpressionType::binary_operator_reference:
        case ExpressionType::prefix_operator_reference:
        case ExpressionType::postfix_operator_reference:
            return "operator " + reference_value()->name_string();
        case ExpressionType::type_operator_reference:
            return "type operator " + reference_value()->name_string();
        case ExpressionType::type_constructor_reference:
            return "reference to type constructor " + reference_value()->name_string();
   
        case ExpressionType::type_field_name:
            return "named field" + string_value();

        case ExpressionType::user_error:
            return "broken expession (syntax error)";
        
        case ExpressionType::compiler_error:
            return "broken expression (compiler error)";

        case ExpressionType::partial_binop_call_both:
            assert(false && "not implemented");

        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_call:
        case ExpressionType::call: {
            std::stringstream output{};
            ReverseParser{&output} << "Call expression " << *this;
            return output.str();
        }

        case ExpressionType::lambda:
            return "lambda expression";

        case ExpressionType::ternary_expression:
            return "ternary expression";

        case ExpressionType::missing_arg:
            return "incomplete partial application missing argument of type " + type->name_string();

        case ExpressionType::deleted:
            return "deleted expession";

        case ExpressionType::minus_sign:
            return "-";

        case ExpressionType::partially_applied_minus:
            return "-( " + std::get<Expression*>(value)->log_message_string() + " )";
    }
}

optional<Expression*> Expression::cast_to(CompilationState& state, const Type* target_type, 
    const SourceLocation& type_declaration_location) {
    
    using Log = LogInContext<LogContext::type_checks>;

    switch (expression_type) {
        case NON_CASTABLE_EXPRESSION:
            Log::debug("Expression " + log_message_string() + " in not castable", type_declaration_location);
            return nullopt;

        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
        case ExpressionType::known_value:
            if (type->cast_to(target_type, *this)) {
                Log::debug_extra("Casted expression " + log_message_string() + " to " + 
                    target_type->name_string(), type_declaration_location);
                return this;
            }
                
            Log::debug("Could not cast " + log_message_string() + " to " + target_type->name_string(), 
                type_declaration_location);
            return nullopt;

        case ExpressionType::missing_arg:
            type = target_type;
            return this;

        case ExpressionType::known_value_reference:
            // copy and cast

        case ExpressionType::reference:
        case ExpressionType::identifier:
        case ExpressionType::termed_expression:
        
        case ExpressionType::partially_applied_minus:
        case ExpressionType::call:
        case ExpressionType::partial_call:
        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_binop_call_both:
            return wrap_in_runtime_cast(state, target_type, type_declaration_location);

        case ExpressionType::lambda:
        case ExpressionType::ternary_expression:
            assert(false && "not implemented");
    }
}


optional<Expression*> Expression::cast_to(CompilationState& state, const Type* target_type) {
    return this->cast_to(state, target_type, this->location);
}

optional<Expression*> Expression::wrap_in_runtime_cast(CompilationState& state, const Type* target_type, 
    const SourceLocation& type_declaration_location) {
    
    assert(is_castable_expression() && "wrap_in_runtime_cast called on not a castable expression");

    auto runtime_cast = state.find_runtime_cast(type, target_type);

    if (!runtime_cast) {
        Log::debug("Could not cast " + log_message_string() + " to " + target_type->name_string(), 
            type_declaration_location);
        return nullopt;
    }

    optional<Expression*> wrapper = Expression::call(state, *runtime_cast, {this},type_declaration_location);

    if (!wrapper) {
        Log::debug("Could not create cast wrapper for " + log_message_string() + " to type " + 
            target_type->name_string(), type_declaration_location);
        return nullopt;
    }

    return wrapper;
}

} // namespace Maps