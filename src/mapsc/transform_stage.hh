#ifndef __TRANSFROMS_HH
#define __TRANSFROMS_HH

#include <span>

#include "mapsc/ast/scope.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/compilation_state.hh"

namespace Maps {

[[nodiscard]] bool run_transforms(CompilationState& state, RT_Scope& scope, RT_Definition& definition);
[[nodiscard]] bool run_transforms(CompilationState& state, RT_Scope& scope, std::span<RT_Definition* const> definitions);

}

#endif