#ifndef __INLINE_HH
#define __INLINE_HH

namespace Maps {

struct Expression;
class Callable;
    
// these run a typecheck, so be sure not to call them from typechecks to avoid infinite recursion
[[nodiscard]] bool inline_call(Expression& call, Callable& callable);
[[nodiscard]] bool inline_call(Expression& call); 

[[nodiscard]] bool substitute_value_reference(Expression& reference, Callable& callable);
[[nodiscard]] bool substitute_value_reference(Expression& reference);

} // namespace Maps

#endif