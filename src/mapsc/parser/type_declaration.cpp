#include "type_declaration.hh"

#include <cassert>

#include "mapsc/ast/ast.hh"

namespace Maps {

bool handle_BTD_field_names(std::vector<Expression*>& possible_BTDs) {
    for (Expression* expression: possible_BTDs) {
        switch (expression->expression_type) {
            case ExpressionType::termed_expression:
                if (!handle_possible_BTD(expression))
                    return false;

            default:
                continue;
        }
    }

    return true;
}

bool handle_possible_BTD(Expression* possible_BTD) {
    assert(possible_BTD->expression_type == ExpressionType::termed_expression && 
        "handle_possible_BTD called with not a termed expression");

    // TODO: figure this out
    // it might be we need to 
    return true;

    // if (possible_BTD->terms().size() <= 1)
    //     return true;

    // for (Expression* term: possible_BTD->terms()) {
    //     if (!term->is_allowed_in_type_declaration())
    //         return true;

    //     switch (term->expression_type) {
    //         case ExpressionType::type_identifier:
    //         case ExpressionType::type_operator_identifier:

    //         case ExpressionType::termed_expression:
    //         case ExpressionType::identifier:
    //     }
    // }
}   

bool is_valid_type_declaration(Expression* expression) {
    assert(false && "not implemented");
}
} // namespace Maps