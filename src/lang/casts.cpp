#include "casts.hh"

#include <cassert>

#include "../logging.hh"

using Logging::log_error;

namespace Maps {

bool static_cast_(Expression* expression, const Type* target_type, bool const_value = false) {
    switch (expression->expression_type) {
        case ExpressionType::numeric_literal:
            break;

        case ExpressionType::string_literal:
            
        case ExpressionType::call:

        case ExpressionType::missing_arg:

        case ExpressionType::reference:
        case ExpressionType::identifier:

        case ExpressionType::termed_expression:
            // maybe parse it and then?
            // but we don't know if the identifiers are there yet?

        case ExpressionType::deleted:
        case ExpressionType::empty:
        case ExpressionType::not_implemented:
        case ExpressionType::operator_e:
        case ExpressionType::operator_ref:
        case ExpressionType::syntax_error:
        case ExpressionType::tie:
            log_error(expression->location, "Trying to apply type specifier to an invalid expression type");
            assert(false);
            return false;
    }
}

} // namespace Maps
