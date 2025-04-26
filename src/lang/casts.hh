#ifndef __CASTS_HH
#define __CASTS_HH

#include "type.hh"
#include "ast_node.hh"

namespace Maps {

// takes an expression and tries to cast it IN PLACE into target type
// argument const value should be used if this is a new expression created from a known const value
// like a literal
bool cast(Expression* expression, const Type* target_type, bool const_value = false);

} // namespace Maps

#endif