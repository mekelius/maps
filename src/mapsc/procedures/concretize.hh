#ifndef __CONCRETIZE_HH
#define __CONCRETIZE_HH

namespace Maps {

class CompilationState;
struct Expression;
class RT_Definition;

// tries to cast all types into concrete types
// Also performs variable substitution (if allowed)
bool concretize(CompilationState& state, RT_Definition& definition);
bool concretize(CompilationState& state, Expression& expression);

} // namespace Maps
#endif