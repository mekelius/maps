#ifndef __TRANSFROMS_HH
#define __TRANSFROMS_HH

#include <span>

#include "mapsc/ast/scope.hh"

namespace Maps {

class CompilationState;
class DefinitionBody;

[[nodiscard]] bool run_transforms(CompilationState& state, Scope& scope, DefinitionBody& definition);
[[nodiscard]] bool run_transforms(CompilationState& state, Scope& scope, std::span<DefinitionBody* const> definitions);
[[nodiscard]] bool run_transforms(CompilationState& state, Scope& scope, std::span<DefinitionHeader* const> definitions);

}

#endif