#ifndef __BUILTIN_HH
#define __BUILTIN_HH

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/definition_body.hh"

namespace Maps {

using Builtin = DefinitionHeader;

// DefinitionHeader has a convenience constructor for creating these
// Probably bad design but whatever
using BuiltinExternal = DefinitionHeader;

using BuiltinValue = std::variant<maps_Boolean, maps_String, maps_Int, maps_Float>;

// class BuiltinBody: public DefinitionBody {
// public:
//     // Builtin(const std::string& name, Statement&& statement, const Type& type);
//     Builtin(const std::string& name, BuiltinValue value, const Type* type)
//     :DefinitionHeader(name, type, BUILTIN_SOURCE_LOCATION), value_(value) {}

// private:
//     BuiltinValue value_;
// };

Builtin create_builtin();

} // namespace Maps

#endif