#ifndef __NAME_RESOLUTION_HH
#define __NAME_RESOLUTION_HH

#include <span>
#include <vector>

#include "mapsc/ast/scope.hh"

namespace Maps {

class CompilationState;
struct Expression;

using Scopes = std::pair<std::vector<CT_Scope*>, std::vector<RT_Scope*>>;

bool resolve_identifiers(CompilationState& state, const Scopes& scopes, 
    std::vector<Expression*>& unresolved_identifiers);
bool resolve_identifiers(CompilationState& state, const std::vector<RT_Scope*>& scopes, 
    std::vector<Expression*>& unresolved_identifiers);
bool resolve_identifiers(CompilationState& state, const std::vector<CT_Scope*>& scopes, 
    std::vector<Expression*>& unresolved_identifiers);

} //namespace Maps

#endif