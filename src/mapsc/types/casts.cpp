#include "casts.hh"

#include <cassert>
#include <optional>
#include <string>
#include <variant>
#include <cstring>

extern "C" {

#include "libmaps.h"

}

#include "common/maps_datatypes.h"
#include "mapsc/logging.hh"

#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/expression_properties.hh"


using std::nullopt;


namespace Maps {

using Log = LogInContext<LogContext::type_casts>;

namespace {

template<typename T>
void cast_value(Expression& expression, const Type* type, T value) {
    assert(is_constant_value(expression) && "Trying to compile-time cast a runtime value");
    expression.value = value;
    expression.expression_type = ExpressionType::known_value;
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

    if (*target_type == String) {
        cast_value<std::string>(expression, &String, std::to_string(int_value));
        return true;
    }

    if (*target_type == MutString) {
        maps_MutString* mut_string_value = to_MutString_Int(int_value);
        cast_value<maps_MutString>(expression, &MutString, *mut_string_value);
    }

    return false;
}

bool cast_from_Float(const Type* target_type, Expression& expression) {
    double double_value = std::get<maps_Float>(expression.value);

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
        if (!CT_to_Int_String(expression.string_value().data(), &result))
            return false;

        cast_value<int>(expression, &Int, result);
        return true;
    }

    if (*target_type == Float) {
        maps_Float result;
        if (!CT_to_Float_String(expression.string_value().data(), &result))
            return false;

        cast_value<maps_Float>(expression, &Float, result);
        return true;
    }

    if (*target_type == MutString) {
        auto old_value = expression.string_value();

        // include the null terminator
        auto new_str = malloc(old_value.size() + 1);
        maps_MutString value{static_cast<char*>(new_str), 
            static_cast<maps_UInt>(old_value.size()), old_value.size() + 1};

        std::memcpy(new_str, old_value.data(), old_value.size() + 1);

        cast_value<maps_MutString>(expression, &MutString, value);
        return true;
    }

    Log::error(expression.location) << "Cannot convert String to " << *target_type << Endl;
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
        cast_value<std::string>(expression, &String, expression.string_value().data());
        return true;
    }

    if (*target_type == Int) {
        if (!std::holds_alternative<std::string>(expression.value)) {
            Log::compiler_error(expression.location) <<
                "Tried to cast NumberLiteral " << expression << 
                " to Int, but it did not hold a NumberLiteral value (string)" << Endl;

            Log::compiler_error(expression.location) << "Instead held " << 
                log_representation(expression.value) << " of index " << expression.value.index() << Endl;
            assert(false && "Numberliteral didn't hold a string");            
            return false;
        }

        maps_Int result;
        if (!CT_to_Int_String(expression.string_value().data(), &result)) {
            Log::debug(expression.location) 
                << expression << " is not an integral numberliteral" << Endl;
            return false;
        }

        cast_value<maps_Int>(expression, &Int, result);
        return true;
    }

    if (*target_type == MutString) {
        maps_Int int_result;
        if (CT_to_Int_String(expression.string_value().data(), &int_result)) {
            cast_value<maps_Int>(expression, &Int, int_result);
            return cast_from_Int(&MutString, expression);
        }

        Log::warning(NO_SOURCE_LOCATION) <<
            "Casts from NumberLiteral to MutString only implemented for integral values";
        return false;
    }

    if (*target_type == Float) {
        maps_Float result;
        if (!CT_to_Float_String(expression.string_value().data(), &result))
            return false;

        cast_value<maps_Float>(expression, &Float, result);
        return true;
    }

    return false;
}

bool cast_from_MutString(const Type* target_type, Expression& expression) {
    if (*target_type == String) {
        auto [data, length, mem_size] = std::get<maps_MutString>(expression.value);

        cast_value<std::string>(expression, &String, std::string{data, mem_size - 1});

        return true;
    }

    Log::compiler_error(expression.location) << "Mut string casts not implemented" << Endl;
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
