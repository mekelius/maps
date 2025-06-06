#ifndef __CONCRETIZE_HH
#define __CONCRETIZE_HH

#include <vector>

namespace Maps {

class CompilationState;
struct Expression;
struct Statement;
class Type;
class DefinitionBody;

// tries to cast all types into concrete types
// Also performs variable substitution (if allowed)
bool concretize(CompilationState& state, DefinitionBody& definition);
bool concretize(CompilationState& state, Expression& expression);
bool concretize(CompilationState& state, std::vector<const Type*>& potential_return_types, 
    Statement& statement);

} // namespace Maps
#endif