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
#include "mapsc/builtins.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/procedures/reverse_parse.hh"

using std::optional, std::nullopt, std::to_string;

namespace Maps {

using Log = LogNoContext;

// ----- STATIC METHODS -----

std::string Expression::value_to_string(const KnownValue& value) {
    return std::visit(overloaded{
        [](const std::string& value)->std::string { return value; },
        [](const maps_MutString& value)->std::string { return std::string{value.data, value.mem_size}; },
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
        [](Expression* expression) { return "@reference to@ " + expression->log_message_string(); },
        [](Definition* target)->std::string { return "@reference to " + target->name_string() + "@"; },                       
        [](const Type* type)->std::string { return "@type: " + type->name_string() + "@"; },
        [](TermedExpressionValue value) { 
            return "@unparsed termed expression of length " + to_string(value.terms.size()) + "@"; },
        [](CallExpressionValue)->std::string { return "@call@"; },
        // [](LambdaExpressionValue)->std::string { return "@lambda@"; },
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
        case ExpressionType::layer2_expression:
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

        case ExpressionType::layer2_expression:
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

        case ExpressionType::known_value_reference:
            return "reference to known value " + reference_value()->name_string();
        case ExpressionType::reference:
            return "reference to " + reference_value()->name_string();
        case ExpressionType::type_reference:
            return "reference to type " + type_reference_value()->name_string();
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

        // case ExpressionType::lambda:
        //     return "lambda expression";

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

std::string_view Expression::expression_type_string_view() const {
    switch (expression_type) {
        case ExpressionType::string_literal: return "string_literal";
        case ExpressionType::numeric_literal: return "numeric_literal";
        case ExpressionType::known_value: return "known_value";
        case ExpressionType::identifier: return "identifier";
        case ExpressionType::operator_identifier: return "operator_identifier";
        case ExpressionType::type_operator_identifier: return "type_operator_identifier";
        case ExpressionType::type_identifier: return "type_identifier";
        case ExpressionType::type_construct: return "type_construct";
        case ExpressionType::type_argument: return "type_argument";
        case ExpressionType::minus_sign: return "minus_sign";
        case ExpressionType::reference: return "reference";
        case ExpressionType::known_value_reference: return "known_value_reference";
        case ExpressionType::binary_operator_reference: return "binary_operator_reference";
        case ExpressionType::prefix_operator_reference: return "prefix_operator_reference";
        case ExpressionType::postfix_operator_reference: return "postfix_operator_reference";
        case ExpressionType::type_reference: return "type_reference";
        case ExpressionType::type_operator_reference: return "type_operator_reference";
        case ExpressionType::type_constructor_reference: return "type_constructor_reference";
        case ExpressionType::type_field_name: return "type_field_name";
        case ExpressionType::layer2_expression: return "termed_expression";
        case ExpressionType::user_error: return "user_error";
        case ExpressionType::compiler_error: return "compiler_error";
        case ExpressionType::call: return "call";
        case ExpressionType::partial_call: return "partial_call";
        case ExpressionType::partial_binop_call_left: return "partial_binop_call_left";
        case ExpressionType::partial_binop_call_right: return "partial_binop_call_right";
        case ExpressionType::partial_binop_call_both: return "partial_binop_call_both";
        case ExpressionType::partially_applied_minus: return "partially_applied_minus";
        case ExpressionType::missing_arg: return "missing_arg";
        // case ExpressionType::lambda: return "lambda";
        case ExpressionType::ternary_expression: return "ternary_expression";
        case ExpressionType::deleted: return "deleted"; 
    }
}

} // namespace Maps