#include "type_defs.hh"

#include <cassert>

namespace Maps {

namespace TypeConstructors {

const Type* ImpureFunctionConstructor::make_type(const std::vector<TypeConstructor::TypeArg>& args, 
    std::string* name) {
    assert(false && "not implemented");    
}

PureFunctionConstructor::PureFunctionConstructor()
:TypeConstructor("->", TypeConstructor::ARITY_N) {
    
}

const Type* PureFunctionConstructor::make_type(const std::vector<TypeConstructor::TypeArg>& args, 
    std::string* name) {
    assert(false && "not implemented");
}

ImpureFunctionConstructor::ImpureFunctionConstructor()
:TypeConstructor("=>", TypeConstructor::ARITY_N) {
    
}

} // namespace TypeConstructors

} // namespace Maps