#include "builtin.hh"

#include <string_view>

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/definition_body.hh"
#include "mapsc/types/type.hh"

namespace Maps {

Builtin create_builtin(std::string_view name, BuiltinValue value, const Type* type) {
    return DefinitionHeader{DefinitionType::builtin, std::string{name}, type, BUILTIN_SOURCE_LOCATION};
}

} // namespace Maps