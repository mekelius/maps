#include "name_resolution.hh"

#include <cassert>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/reference.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/logging.hh"

#include "mapsc/source_location.hh"
#include "mapsc/types/type_store.hh"

using std::optional, std::nullopt;


namespace Maps {


} // namespace Maps