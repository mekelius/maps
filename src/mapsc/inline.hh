#ifndef __INLINE_HH
#define __INLINE_HH

struct Expression;
struct Callable;

namespace Maps {

// these run a typecheck, so be sure not to call them from typechecks to avoid infinite recursion
[[nodiscard]] bool inline_call(Expression& expression); 
[[nodiscard]] bool inline_call(Expression& expression, Callable& callable);

[[nodiscard]] bool substitute_value_reference(Expression& expression); 
[[nodiscard]] bool substitute_value_reference(Expression& expression, Callable& callable);

} // namespace Maps

#endif