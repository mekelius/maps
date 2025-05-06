#include "casts.hh"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <optional>

#include "mapsc/logging.hh"

#include "mapsc/types/type_defs.hh"

#include "mapsc/ast/ast_node.hh"

using std::nullopt;
using Logging::log_error;

namespace Maps {

namespace {

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

std::optional<double> string_to_double(std::string str) {
    errno = 0;
    char* end = nullptr;
    int result = std::strtod(str.c_str(), &end);

    if (*end)
        return std::nullopt;

    if (errno != 0 && result == 0)
        return std::nullopt;

    return result;
}

template<typename T>
void cast_value(Expression& expression, const Type* type, T value) {
    expression.value = value;
    expression.expression_type = ExpressionType::value;
    expression.type = type;
}

} // anonymous namespace

bool not_castable(const Type*, Expression&) {
    return false;
}

bool cast_from_Int(const Type* target_type, Expression& expression) {
    assert(false && "not implemented");
    if (*target_type == Float) {
        return true;
    }

    if (*target_type == Number) {
        return true;
    }

    if (*target_type == String) {
        return true;
    }

    return false;
}

bool cast_from_Float(const Type* target_type, Expression& expression) {
    assert(false && "not implemented");

    if (*target_type == String) {
        return true;
    }

    return false;
}

bool cast_from_Number(const Type* target_type, Expression& expression) {
    if (*target_type == String)
        return true;

    if (*target_type == Int)
        return cast_from_String(&Int, expression);
    
    if (*target_type == Float)
        return cast_from_String(&Float, expression);

    return false;
}

bool cast_from_String(const Type* target_type, Expression& expression) {
    if (*target_type == String)
        return true;

    if (*target_type == Int) {
        auto result = string_to_int(expression.string_value());
        if (!result)
            return false;

        cast_value<int>(expression, &Int, *result);
        return true;
    }

    if (*target_type == Float) {
        auto result = string_to_double(expression.string_value());
        if (!result)
            return false;

        cast_value<double>(expression, &Int, *result);
        return true;
    }

    assert(false && "not implemented");
    return false;
}

bool cast_from_Boolean(const Type* target_type, Expression& expression) {
    assert(false && "not implemented");

    if (*target_type == String)
        return true;

    return false;
}

bool cast_from_NumberLiteral(const Type* target_type, Expression& expression) {
    if (*target_type == String)
        return true;

    if (*target_type == Int) {
        if (!std::holds_alternative<std::string>(expression.value))
            return false;

        std::optional<int> result = string_to_int(expression.string_value());
        if (!result)
            return false;

        cast_value<int>(expression, &Int, *result);
        return true;
    }

    assert(false && "not implemented");

    if (*target_type == Float) {
        
    }

    if (*target_type == Number) {

    }

    return false;
}

// ----- CONCRETIZATION FUNCTIONS -----

bool is_concrete(Expression& expression) {
    return true;
}

bool not_concretizable(Expression& expression) {
    return false;
}

bool concretize_Number(Expression& expression) {
    if (cast_from_Number(&Int, expression))
        return true;

    if (cast_from_Number(&Float, expression))
        return true;

    return false;
}

bool concretize_NumberLiteral(Expression& expression) {
    if (cast_from_String(&Int, expression))
        return true;

    if (cast_from_String(&Float, expression))
        return true;

    return false;
}

} // namespace Maps
