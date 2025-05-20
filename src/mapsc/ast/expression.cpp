#include "expression.hh"

#include <memory>
#include <span>
#include <cassert>
#include <sstream>

#include "mapsc/logging.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/procedures/reverse_parse.hh"


using std::optional, std::nullopt, std::to_string;

namespace Maps {

using Log = LogNoContext;

// ----- EXPRESSION -----

bool Expression::is_reduced_value() const {
    switch (expression_type) {
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
        case ExpressionType::value:
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
        return std::get<Definition*>(value)->to_string();
    }
    return std::get<std::string>(value);
}

bool Expression::is_illegal() const {
    switch (expression_type) {
        case ExpressionType::deleted:
        case ExpressionType::not_implemented:
        case ExpressionType::syntax_error:
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
    return true;
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
            return std::get<TermedExpressionValue>(value).to_string();

        case ExpressionType::string_literal:
            return "string literal \"" + string_value() + "\"";

        case ExpressionType::numeric_literal:
            return "numeric literal +" + string_value();
    
        case ExpressionType::value:
            return "value expression of type " + type->to_string();
        
        case ExpressionType::identifier:
            return "identifier " + string_value();

        case ExpressionType::operator_identifier:
            return "operator " + string_value();
        case ExpressionType::type_operator_identifier:
            return "type operator " + string_value();
        case ExpressionType::type_identifier:
            return "type identifier " + string_value();

        case ExpressionType::reference:
            return "reference to " + reference_value()->to_string();
        case ExpressionType::type_reference:
            return "reference to type " + reference_value()->to_string();
        case ExpressionType::binary_operator_reference:
        case ExpressionType::prefix_operator_reference:
        case ExpressionType::postfix_operator_reference:
            return "operator " + reference_value()->to_string();
        case ExpressionType::type_operator_reference:
            return "type operator " + reference_value()->to_string();
        case ExpressionType::type_constructor_reference:
            return "reference to type constructor " + reference_value()->to_string();
   
        case ExpressionType::type_field_name:
            return "named field" + string_value();

        case ExpressionType::syntax_error:
            return "broken expession (syntax error)";
        
        case ExpressionType::compiler_error:
            return "broken expression (compiler error)";

        case ExpressionType::not_implemented:
            return "nonimplemented expression";

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
            return "incomplete partial application missing argument of type " + type->to_string();

        case ExpressionType::deleted:
            return "deleted expession";

        case ExpressionType::minus_sign:
            return "-";

        case ExpressionType::partially_applied_minus:
            return "-( " + std::get<Expression*>(value)->log_message_string() + " )";
    }
}

std::string TermedExpressionValue::to_string() const {
    std::stringstream output{"expression "};
    output << this;
    return output.str();
}

} // namespace Maps