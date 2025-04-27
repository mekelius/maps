#include "casts.hh"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <optional>

#include "../logging.hh"

#include "type_defs.hh"

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
    assert(false && "not implemented");

    if (*target_type == String) {
        return true;
    }

    if (*target_type == Int) {

    }

    if (*target_type == Float) {
        
    }

    if (*target_type == Number) {

    }

    return false;
}

} // namespace Maps
