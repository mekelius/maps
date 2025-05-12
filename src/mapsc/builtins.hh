#ifndef __BUILTINS_HH
#define __BUILTINS_HH

#include <memory>

namespace Maps {

class Scope;
    
// // automatically creates an identifier and a global callable for the builtin
// constexpr Callable* create_builtin(const std::string& name, const Type& type);

// // TODO: move these somewhere else
// // TODO: add a constraint here that T has to be Maps type
// template <typename T>
// constexpr Callable* create_builtin_value(const std::string& name, T value, const Type& type) {
//     builtins_.push_back(std::make_unique<Builtin>(name, &type));
//     Builtin* builtin = builtins_.back().get();

//     assert(!builtins_scope_->identifier_exists(name)
//         && "tried to redefine an existing builtin");
    
//     return *builtins_scope_->create_callable(name, builtin);
// }

// constexpr Callable* create_builtin_binary_operator(const std::string& name, const Type& type, 
//     Precedence precedence, Associativity Associativity = Associativity::left);
// constexpr Callable* create_builtin_unary_operator(const std::string& name, const Type& type, 
//     UnaryFixity fixity = UnaryFixity::prefix);

// TODO: how could this be made constexpr

const Scope* get_builtins();

} // namespace Maps

#endif