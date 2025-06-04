#include "scope.hh"

#include <map>
#include <optional>
#include <variant>
#include <vector>
#include <cassert>
#include <span>

#include "mapsc/source_location.hh"
#include "mapsc/logging.hh"
#include "mapsc/types/type.hh"
#include "mapsc/ast/definition.hh"

namespace Maps {

using Log_resolution = LogInContext<LogContext::name_resolution>;
using Log_creation = LogInContext<LogContext::definition_creation>;

std::optional<DefinitionHeader*> Scope::create_identifier(DefinitionHeader* node) {
    auto name = node->name_string();
    if (identifier_exists(name)) {
        Log_creation::error(node->location()) << "Attempting to redefine identifier " << name << Endl;
        return std::nullopt;
    }

    identifiers_.insert(std::pair<std::string, DefinitionHeader*>{name, node});
    identifiers_in_order_.push_back(node);
    
    Log_creation::debug_extra(node->location()) << "Created identifier " << name << Endl;

    assert(identifier_exists(node->name_) && "created identifier doesn't exist");

    return node;
}

} // namespace Maps