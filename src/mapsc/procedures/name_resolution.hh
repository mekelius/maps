#ifndef __NAME_RESOLUTION_HH
#define __NAME_RESOLUTION_HH

#include <span>
#include <vector>

namespace Maps {

class CompilationState;
class Scope;
struct Expression;

bool resolve_identifiers(CompilationState& state, const std::vector<Scope*>& scopes, 
    std::vector<Expression*>& unresolved_identifiers);

} //namespace Maps

#endif