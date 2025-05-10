#include "expression.hh"

#include <cassert>
#include <sstream>

#include "mapsc/logging.hh"
#include "mapsc/procedures/reverse_parse.hh"

using Maps::GlobalLogger::log_error;

namespace Maps {

// ----- EXPRESSION -----

DeferredBool Expression::has_native_representation() {    
    return type->is_castable_to_native();
}

std::vector<Expression*>& Expression::terms() {
    return std::get<TermedExpressionValue>(value).terms;
}
CallExpressionValue& Expression::call_value() {
    return std::get<CallExpressionValue>(value);
}
Callable* Expression::reference_value() const {
    return std::get<Callable*>(value);
}

bool Expression::is_partial_call() const {
    if (expression_type != ExpressionType::call)
        return false;

    auto [callee, args] = std::get<CallExpressionValue>(value);

    if (args.size() < callee->get_type()->arity())
        return true;

    for (auto arg: args) {
        if (arg->expression_type == ExpressionType::missing_arg)
            return true;
    }

    return false;
}

bool Expression::is_reduced_value() const {
    switch (expression_type) {
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
        case ExpressionType::reference:
        case ExpressionType::call:
        case ExpressionType::value:
            return true;

        default:
            return false;
    }
}

void Expression::mark_not_type_declaration() {
    if (expression_type != ExpressionType::termed_expression)
        return;

    std::get<TermedExpressionValue>(value).is_type_declaration = DeferredBool::false_;
}

DeferredBool Expression::is_type_declaration() {
    switch (expression_type) {
        case ExpressionType::termed_expression:
            return std::get<TermedExpressionValue>(value).is_type_declaration;

        case ExpressionType::type_identifier:
        case ExpressionType::type_construct:
        case ExpressionType::type_reference:
        case ExpressionType::type_constructor_reference:
            return DeferredBool::true_;

        default:
            return DeferredBool::false_;
    }
}

// TODO: clean this up
const std::string& Expression::string_value() const {
    if (std::holds_alternative<Callable*>(value)) {
        // !!! this will cause crashes when lambdas come in
        return std::get<Callable*>(value)->name;
    }
    return std::get<std::string>(value);
}

bool Expression::is_literal() const {
    switch (expression_type) {
        case ExpressionType::numeric_literal:
        case ExpressionType::string_literal:
            return true;

        default:
            return false;
    }
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

bool Expression::is_reference() const {
    switch (expression_type) {
        case ExpressionType::reference:
        case ExpressionType::type_reference:
        case ExpressionType::operator_reference:
        case ExpressionType::type_operator_reference:
        case ExpressionType::type_constructor_reference:
            return true;

        default:
            return false;
    }
}

bool Expression::is_identifier() const {
    switch (expression_type) {
        case ExpressionType::identifier:
        case ExpressionType::type_identifier:
        case ExpressionType::operator_identifier:
        case ExpressionType::type_operator_identifier:
            return true;

        default:
            return false;
    }
}

bool Expression::is_ok_in_layer2() const {
    return (!is_identifier() && !is_illegal());
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
            return "reference to " + reference_value()->name;
        case ExpressionType::type_reference:
            return "reference to type " + reference_value()->name;
        case ExpressionType::operator_reference:
            return "operator " + reference_value()->name;
        case ExpressionType::type_operator_reference:
            return "type operator " + reference_value()->name;
        case ExpressionType::type_constructor_reference:
            return "reference to type constructor " + reference_value()->name;
   
        case ExpressionType::type_field_name:
            return "named field" + string_value();

        case ExpressionType::syntax_error:
            return "broken expression";
        
        case ExpressionType::not_implemented:
            return "nonimplemented expression";

        case ExpressionType::call: {
            std::stringstream output{"call expression "};
            output << this;
            return output.str();
        }

        case ExpressionType::missing_arg:
            return "incomplete partial application missing argument of type " + type->to_string();

        case ExpressionType::deleted:
            return "deleted expession";

        default:
            assert(false && "unknown expression type");
    }
}

std::string TermedExpressionValue::to_string() const {
    std::stringstream output{"expression "};
    output << this;
    return output.str();
}

} // namespace Maps