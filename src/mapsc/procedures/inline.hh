#ifndef __INLINE_HH
#define __INLINE_HH

namespace Maps {

struct Expression;
class Definition;
class RT_Definition;
    
bool inline_and_substitute(RT_Definition& definition);

// these run a typecheck, so be sure not to call them from typechecks to avoid infinite recursion
[[nodiscard]] bool inline_call(Expression& call, const Definition& definition);
[[nodiscard]] bool inline_call(Expression& call); 

[[nodiscard]] bool substitute_value_reference(Expression& reference, const Definition& definition);
[[nodiscard]] bool substitute_value_reference(Expression& reference);

} // namespace Maps

#endif