#ifndef __NAME_RESOLUTION_HH
#define __NAME_RESOLUTION_HH

#include <span>
#include <vector>

#include "mapsc/ast/scope.hh"

namespace Maps {

class CompilationState;
struct Expression;

using Scopes = std::pair<std::span<const CT_Scope* const>, std::span<const RT_Scope* const>>;


bool resolve_identifiers(CompilationState& state, const Scopes& scopes, 
    std::vector<Expression*>& unresolved_identifiers);

bool resolve_identifiers(CompilationState& state, std::span<const RT_Scope* const> scopes, 
    std::vector<Expression*>& unresolved_identifiers);
bool resolve_identifiers(CompilationState& state, std::span<const CT_Scope* const> scopes, 
    std::vector<Expression*>& unresolved_identifiers);
bool resolve_identifiers(CompilationState& state, const CT_Scope& ct_scope, 
    const RT_Scope& rt_scope, std::vector<Expression*>& unresolved_identifiers);

bool resolve_identifiers(CompilationState& state, const RT_Scope& scope, 
    std::vector<Expression*>& unresolved_identifiers);
bool resolve_identifiers(CompilationState& state, 
    std::vector<Expression*>& unresolved_identifiers);

} //namespace Maps

#endif