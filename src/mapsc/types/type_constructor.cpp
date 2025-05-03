#include "type_constructor.hh"

#include <cassert>

#include "type.hh"

namespace Maps {

TypeConstructor::TypeConstructor(const std::string& name, int arity)
:name_(name), arity_(arity) {}

ImpureFunctionConstructor::ImpureFunctionConstructor()
:TypeConstructor("=>", TypeConstructor::ARITY_N) {}

const Type* ImpureFunctionConstructor::make_type(TypeRegistry& type_registry, 
    std::vector<TypeConstructor::TypeArg>&& args, std::string* name) {
    assert(false && "not implemented");    
}

PureFunctionConstructor::PureFunctionConstructor()
:TypeConstructor("->", TypeConstructor::ARITY_N) {}

const Type* PureFunctionConstructor::make_type(TypeRegistry& type_registry, 
    std::vector<TypeConstructor::TypeArg>&& args_, std::string* name) {

    auto args = args_;

    assert(args.size() > 0 && "PureFuntionConstructor::make_type called with no args");

    Type* return_type = args.back();
    args.pop_back();
    type_registry.get_function_type(return_type, args.at(0), true);
    return nullptr;
}

} // namespace Maps