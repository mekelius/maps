#ifndef __BUILTIN_HH
#define __BUILTIN_HH

#include <string_view>

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/ast/definition_body.hh"

namespace Maps {

class Type;
using BuiltinValue = std::variant<maps_Boolean, maps_String, maps_Int, maps_Float>;

struct Builtin {
    DefinitionHeader header;
    BuiltinValue value;
};

// DefinitionHeader has a convenience constructor for creating these
// Probably bad design but whatever
using BuiltinExternal = DefinitionHeader;


// class BuiltinBody: public DefinitionBody {
// public:
//     // Builtin(const std::string& name, Statement&& statement, const Type& type);
//     Builtin(const std::string& name, BuiltinValue value, const Type* type)
//     :DefinitionHeader(name, type, BUILTIN_SOURCE_LOCATION), value_(value) {}

// private:
//     BuiltinValue value_;
// };

Builtin create_builtin(std::string_view name, BuiltinValue value, const Type* type);

consteval Builtin create_builtin_known_value(std::string_view name, BuiltinValue value) {
    return {
        DefinitionHeader{DefinitionType::builtin, name, &Hole, BUILTIN_SOURCE_LOCATION},
        value
    };
}

consteval Builtin create_builtin_known_value(std::string_view name, BuiltinValue value, const Type* type) {
    return {
        DefinitionHeader{DefinitionType::builtin, name, type, BUILTIN_SOURCE_LOCATION},
        value
    };
}

} // namespace Maps

#endif