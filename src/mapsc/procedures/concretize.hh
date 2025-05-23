#ifndef __CONCRETIZE_HH
#define __CONCRETIZE_HH

namespace Maps {

struct Expression;
class RT_Definition;

// tries to cast all types into concrete types
// Also performs variable substitution (if allowed)
bool concretize(RT_Definition& definition);
bool concretize(Expression& expression);

} // namespace Maps
#endif