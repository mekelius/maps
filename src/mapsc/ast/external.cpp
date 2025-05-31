#include "external.hh"

#include <optional>
#include <string_view>
#include <variant>

#include "common/std_visit_helper.hh"
#include "common/maps_datatypes.h"

#include "mapsc/log_format.hh"
#include "mapsc/source.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/ast_store.hh"

namespace Maps {

External* create_external(AST_Store& ast_store, const std::string& name, const Type* type, 
    const SourceLocation& location) {

    return ast_store.allocate_external(External{name, type, location});
}

} // namespace Maps
