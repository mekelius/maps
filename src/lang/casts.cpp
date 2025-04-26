#include "casts.hh"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <optional>
#include "../logging.hh"

using Logging::log_error;

namespace Maps {

std::optional<int> string_to_int(std::string str) {
    errno = 0;
    char* end = nullptr;
    int result = std::strtol(str.c_str(), &end, 10);

    if (*end)
        return std::nullopt;

    if (errno != 0 && result == 0)
        return std::nullopt;

    return result;
}

bool static_cast_(Expression* expression, const Type* target_type, bool const_value) {
    switch (expression->expression_type) {
        case ExpressionType::numeric_literal:
            if (*target_type == Int) {
                auto value = string_to_int(expression->string_value());

                if (!value) {
                    log_error("value \"" + expression->string_value() + "\" could not be casted to Int");
                    return false;
                }

                expression->type = &Int;
                expression->expression_type = ExpressionType::value;
                expression->value = *value;

                return true;
            }

            assert(false && "not implemented");
            return false;

        case ExpressionType::string_literal:
            if (*target_type == Int) {
                auto value = string_to_int(expression->string_value());

                if (!value) {
                    log_error("value \"" + expression->string_value() + "\" could not be casted to Int");
                    return false;
                }

                expression->type = &Int;
                expression->expression_type = ExpressionType::value;
                expression->value = *value;

                return true;
            }
        case ExpressionType::value:
            
        case ExpressionType::call:

        case ExpressionType::missing_arg:

        case ExpressionType::reference:
        case ExpressionType::identifier:

        case ExpressionType::termed_expression:
            // maybe parse it and then?
            // but we don't know if the identifiers are there yet?
            assert(false && "not implemented");
            return false;

        default:
            log_error(expression->location, "Trying to apply type specifier to an invalid expression type");
            assert(false);
            return false;
    }
}

} // namespace Maps
