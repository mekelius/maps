#ifndef __TRANSFROMS_HH
#define __TRANSFROMS_HH

#include <span>

#include "mapsc/ast/scope.hh"

namespace Maps {

class CompilationState;
class RT_Definition;

[[nodiscard]] bool run_transforms(CompilationState& state, RT_Scope& scope, RT_Definition& definition);
[[nodiscard]] bool run_transforms(CompilationState& state, RT_Scope& scope, std::span<RT_Definition* const> definitions);

}

#endif