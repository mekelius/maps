#ifndef __COERCE_TYPE_HH
#define __COERCE_TYPE_HH

#include <utility>

namespace Maps {

class Type;
struct Expression;
struct Statement;

bool handle_declared_type(Expression& expression, const Type* declared_type);

// the first return value signals whether a coercion was performed,
// the second one whether a return value was found
std::pair<bool, bool> coerce_block_return_type(Statement& statement, const Type* wanted_type);

// Gets the return type of a statement, trying to coerce into the wanted type if given
bool coerce_return_type(Statement& statement, const Type* wanted_type);

} // namespace Maps

#endif