#include "casts.hh"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <optional>

#include "../logging.hh"

#include "type_defs.hh"

#include "ast/ast_node.hh"

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

template<typename T>
void cast_value(Expression* expression, const Type* type, T value) {
    expression->value = value;
    expression->expression_type = ExpressionType::value;
    expression->type = type;
}

} // anonymous namespace

bool not_castable(const Type*, Expression*) {
    return false;
}

bool cast_from_Int(const Type* target_type, Expression* expression) {
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

bool cast_from_Float(const Type* target_type, Expression* expression) {
    assert(false && "not implemented");

    if (*target_type == String) {
        return true;
    }

    return false;
}

bool cast_from_Number(const Type* target_type, Expression* expression) {
    assert(false && "not implemented");
    if (*target_type == String) {
        return true;
    }
    return false;
}

bool cast_from_String(const Type* target_type, Expression* expression) {
    assert(false && "not implemented");

    if (*target_type == String) {
        return true;
    }
    return false;
}

bool cast_from_Boolean(const Type* target_type, Expression* expression) {
    assert(false && "not implemented");

    if (*target_type == String) {
        return true;
    }

    return false;
}

bool cast_from_NumberLiteral(const Type* target_type, Expression* expression) {
    if (*target_type == String) {
        return true;
    }

    if (*target_type == Int) {
        if (!std::holds_alternative<std::string>(expression->value))
            return false;

        std::optional<int> result = string_to_int(expression->string_value());
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

} // namespace Maps
