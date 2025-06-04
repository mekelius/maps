#include "expression.hh"

#include <cassert>
#include <sstream>
#include <variant>
#include <string>

#include "common/std_visit_helper.hh"
#include "mapsc/logging.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/procedures/reverse_parse.hh"

using std::optional, std::nullopt, std::to_string;

namespace Maps {

using Log = LogNoContext;

// ----- STATIC METHODS -----

std::string value_to_string(const KnownValue& value) {
    return std::visit(overloaded{
        [](const std::string& value)->std::string { return value; },
        [](const maps_MutString& value)->std::string { return std::string{value.data, value.mem_size}; },
        [](auto value)->std::string { return std::to_string(value); },
    }, value);
}

// workaround to deal with ambiguity
std::string known_value_to_string(const KnownValue& value) {
    return value_to_string(value);
}

std::string value_to_string(const ExpressionValue& value) {
    return std::visit(overloaded{
        [](std::monostate)->std::string { return "@Undefined expression value@"; },
        [](Expression*)->std::string { return "@reference to expression@"; },
        [](const DefinitionHeader* target)->std::string { return "@reference to " + target->name_string() + "@"; },                       
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

std::string_view Expression::string_value() const {
    if (auto definition = std::get_if<const DefinitionHeader*>(&value)) {
        // !!! this will cause crashes when lambdas come in
        return (*definition)->name_;
    }
    return std::get<std::string>(value);
}

LogStream::InnerStream& Expression::log_self_to(LogStream::InnerStream& ostream) const {
    switch (expression_type) {
        case ExpressionType::type_construct:
            return ostream << "type argument (TODO pretty print)";
            // TOOD:
            // return type_argument_value().to_string();

        case ExpressionType::type_argument:
            return ostream << "type argument (TODO pretty print)";
            // TOOD:
            // return type_argument_value().to_string();

        case ExpressionType::layer2_expression:
            return ostream << value_to_string(value);

        case ExpressionType::known_value:
            return ostream << "value expression " << value_to_string(value);
        
        case ExpressionType::identifier:
            return ostream << "identifier " << string_value();

        case ExpressionType::operator_identifier:
            return ostream << "operator " << string_value();
        case ExpressionType::type_operator_identifier:
            return ostream << "type operator " << string_value();
        case ExpressionType::type_identifier:
            return ostream << "type identifier " << string_value();

        case ExpressionType::known_value_reference:
            ostream << "reference to known value: " << reference_value()->log_representation();
        case ExpressionType::reference:
            return ostream << "reference to: " << reference_value()->log_representation();
        case ExpressionType::type_reference:
            return ostream << "reference to type: " << type_reference_value()->log_representation();
        case ExpressionType::binary_operator_reference:
        case ExpressionType::prefix_operator_reference:
        case ExpressionType::postfix_operator_reference:
            return ostream << "reference to operator: " << reference_value()->log_representation();
        case ExpressionType::type_operator_reference:
            return ostream << "type operator: " << reference_value()->log_representation();
        case ExpressionType::type_constructor_reference:
            return ostream << "reference to type constructor: " << reference_value()->log_representation();
        case ExpressionType::type_field_name:
            return ostream << "named field" << string_value();

        case ExpressionType::user_error:
            return ostream << "broken expession (syntax error)";
        case ExpressionType::compiler_error:
            return ostream << "broken expression (compiler error)";

        case ExpressionType::partial_binop_call_both:
            assert(false && "not implemented");
        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_call:
        case ExpressionType::call:
            // ReverseParser{&logstream} << "Call expression " << *this;
            return ostream;

        // case ExpressionType::lambda:
        //     return "lambda expression";

        case ExpressionType::ternary_expression:
            return ostream << "ternary expression";

        case ExpressionType::missing_arg:
            return ostream << "incomplete partial application missing argument of type: " << type->log_representation();

        case ExpressionType::deleted:
            return ostream << "deleted expession";

        case ExpressionType::minus_sign:
            return ostream << "-";

        case ExpressionType::partially_applied_minus:
            // logstream << "minus sign partially applied to "; 
            // std::get<Expression*>(value)->log_self_to(logstream); 
            return ostream;
    }
}

std::string_view Expression::expression_type_string_view() const {
    switch (expression_type) {
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