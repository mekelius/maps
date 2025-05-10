#include "casts.hh"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <optional>

extern "C" {

#include "corelib.h"

}

#include "mapsc/logging.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/ast_node.hh"
#include "mapsc/ast/expression.hh"

using std::nullopt;
using Maps::GlobalLogger::log_error;

namespace Maps {

namespace {

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
    int int_value = std::get<maps_Int>(expression.value);
    if (*target_type == Float) {
        cast_value<maps_Float>(expression, &Float, static_cast<maps_Float>(int_value));
        return true;
    }

    if (*target_type == Number) {
        cast_value<std::string>(expression, &Number, std::to_string(int_value));
        return true;
    }

    if (*target_type == String) {
        cast_value<std::string>(expression, &String, std::to_string(int_value));
        return true;
    }

    return false;
}

bool cast_from_Float(const Type* target_type, Expression& expression) {
    double double_value = std::get<maps_Float>(expression.value);

    if (*target_type == Number) {
        cast_value<std::string>(expression, &Number, std::to_string(double_value));
        return true;
    }

    if (*target_type == String) {
        cast_value<std::string>(expression, &String, std::to_string(double_value));
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
        maps_Int result;
        if (!const_String_to_Int(expression.string_value().c_str(), &result))
            return false;

        cast_value<int>(expression, &Int, result);
        return true;
    }

    if (*target_type == Float) {
        maps_Float result;
        if (!const_String_to_Float(expression.string_value().c_str(), &result))
            return false;

        cast_value<maps_Float>(expression, &Float, result);
        return true;
    }

    assert(false && "not implemented");
    return false;
}

bool cast_from_Boolean(const Type* target_type, Expression& expression) {
    if (*target_type == String) {
        std::string str_value = std::get<bool>(expression.value) ? "true" : "false";
        cast_value<std::string>(expression, &String, str_value);

        return true;
    }

    return false;
}

bool cast_from_NumberLiteral(const Type* target_type, Expression& expression) {
    if (*target_type == String) {
        cast_value<std::string>(expression, &String, expression.string_value());
        return true;
    }
    
    if (*target_type == Number) {
        cast_value<std::string>(expression, &Number, expression.string_value());
        return true;
    }

    if (*target_type == Int) {
        if (!std::holds_alternative<std::string>(expression.value))
            return false;

        maps_Int result;
        if (!const_String_to_Int(expression.string_value().c_str(), &result))
            return false;

        cast_value<maps_Int>(expression, &Int, result);
        return true;
    }

    assert(false && "not implemented");

    if (*target_type == Float) {
        
    }

    return false;
}

// ----- CONCRETIZATION FUNCTIONS -----

bool is_concrete(Expression& expression) {
    (void) expression;
    return true;
}

bool not_concretizable(Expression& expression) {
    (void) expression;
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
