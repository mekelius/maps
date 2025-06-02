#include "cleanup.hh"

#include "mapsc/logging.hh"

namespace Maps {

using Log = LogNoContext;

bool insert_cleanup(CompilationState& state, Scope& scope, DefinitionBody& definition) {
    Log::warning(NO_SOURCE_LOCATION) << "Cleanup not implemented, all String':s will leak" << Endl;
    return true;
}

} // namespace Maps