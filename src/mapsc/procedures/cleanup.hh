#ifndef __CLEANUP_HH
#define __CLEANUP_HH

#include "mapsc/ast/scope.hh"

namespace Maps {

class CompilationState;
class DefinitionBody;

bool insert_cleanup(CompilationState& state, Scope& scope, DefinitionBody& definition);

}

#endif