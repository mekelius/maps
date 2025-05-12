#ifndef __NAME_RESOLUTION_HH
#define __NAME_RESOLUTION_HH

namespace Maps {

class CompilationState;
struct Expression;

bool resolve_identifiers(CompilationState& state);
bool resolve_identifier(CompilationState& state, Expression* expression);
bool resolve_operator(CompilationState& state, Expression* expression);
bool resolve_type_identifier(CompilationState& state, Expression* expression);

} //namespace Maps

#endif