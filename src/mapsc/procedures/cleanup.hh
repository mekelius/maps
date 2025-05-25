#ifndef __CLEANUP_HH
#define __CLEANUP_HH

#include "mapsc/ast/scope.hh"

namespace Maps {

class CompilationState;
class Definition;

bool insert_cleanup(CompilationState& state, RT_Scope& scope, Definition& definition);

}

#endif