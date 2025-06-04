#ifndef __NAME_RESOLUTION_HH
#define __NAME_RESOLUTION_HH

#include <span>
#include <vector>

#include "mapsc/ast/scope.hh"
#include "mapsc/builtins.hh"

namespace Maps {

class CompilationState;
struct Expression;

bool resolve_identifiers(CompilationState& state, const_Scopes scopes, 
    std::vector<Expression*>& unresolved_identifiers, const BuiltinScope<10>& builtins_ = builtins);

bool resolve_identifiers(CompilationState& state, const Scope& scope, 
    std::vector<Expression*>& unresolved_identifiers, const BuiltinScope<10>& builtins_ = builtins);

} //namespace Maps

#endif