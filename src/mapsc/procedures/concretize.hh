#ifndef __CONCRETIZE_HH
#define __CONCRETIZE_HH

namespace Maps {

struct Expression;

// tries to cast all types into concrete types
// Also performs variable substitution (if allowed)
bool concretize(Expression& expression);

} // namespace Maps
#endif