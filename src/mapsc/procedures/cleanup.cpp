#include "cleanup.hh"

#include "mapsc/logging.hh"

namespace Maps {

using Log = LogNoContext;

bool insert_cleanup(CompilationState& state, Scope& scope, DefinitionBody& definition) {
    Log::warning("Cleanup not implemented, all String':s will leak", NO_SOURCE_LOCATION);
    return true;
}

} // namespace Maps