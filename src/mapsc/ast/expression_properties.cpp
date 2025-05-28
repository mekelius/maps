#include "expression_properties.hh"

#include "mapsc/ast/expression.hh"

namespace Maps {

bool is_allowed_as_arg(const Expression& expression) {
    switch (expression.expression_type) {
        case OPERATOR_EXPRESSION:
        case TYPE_EXPRESSION:
        case ILLEGAL_EXPRESSION:
        case ExpressionType::identifier:
        case ExpressionType::minus_sign:
        case ExpressionType::partially_applied_minus:
        case ExpressionType::layer2_expression:
            return false;

        case ExpressionType::reference:
        case ExpressionType::ternary_expression:
        case ExpressionType::missing_arg:
        case ExpressionType::call:
        case ExpressionType::partial_call:
        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_binop_call_both:
        case ExpressionType::known_value_reference:
        case ExpressionType::known_value:
            return true;
    }
}

bool is_identifier(const Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::identifier:
        case ExpressionType::type_identifier:
        case ExpressionType::operator_identifier:
        case ExpressionType::type_operator_identifier:
            return true;

        default:
            return false;
    }
}

bool is_reference(Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::reference:
        case ExpressionType::type_reference:
        case ExpressionType::prefix_operator_reference:
        case ExpressionType::postfix_operator_reference:
        case ExpressionType::binary_operator_reference:
        case ExpressionType::type_operator_reference:
        case ExpressionType::type_constructor_reference:
            return true;

        default:
            return false;
    }
}

bool is_constant_value(const Expression& expression) {
    switch (expression.expression_type) {
            assert(holds_alternative<std::string>(expression.value) && 
                "Encountered an expression with literal expressiontype but wrong type of body");
            return true;

        case ExpressionType::known_value:
            assert((!holds_alternative<CallExpressionValue>(expression.value)) && 
                "Encountered an expression with expressiontype known value but wrong type of body");
            return true;

        default:
            return false;   
    }
}

bool is_reduced_value(const Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::known_value:
        return true;
        
        case ExpressionType::reference:
        case ExpressionType::call:
        default:
            return false;
    }
}

bool is_illegal(const Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::deleted:
        case ExpressionType::user_error:
        case ExpressionType::compiler_error:
            return true;

        default:
            return false;
    }
}

bool is_ok_in_layer2(const Expression& expression) {
    return !(is_identifier(expression) || 
                is_illegal(expression) || 
                expression.expression_type == ExpressionType::type_operator_reference);
}

bool is_ok_in_codegen(const Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::reference:
        case ExpressionType::call:
            return true;

        default:
            return false;
    }
}

bool is_castable_expression(const Expression& expression) {
    switch (expression.expression_type) {
        case NON_CASTABLE_EXPRESSION:
            return false;

        default:
            return true;
    }
}

bool is_allowed_in_type_declaration(const Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::layer2_expression:
            return std::get<TermedExpressionValue>(expression.value).is_type_declaration != 
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

bool is_partial_call(const Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_call:
            return true;

        case ExpressionType::call:{
            auto [callee, args] = std::get<CallExpressionValue>(expression.value);

            if (args.size() < callee->get_type()->arity())
                return true;

            for (auto arg: args) {
                if (arg->expression_type == ExpressionType::missing_arg)
                    return true;
            }

            return false;
        }
        default:
            return false;
    }
}


} // namespace Maps