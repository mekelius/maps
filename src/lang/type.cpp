#include "type.hh"

#include <cassert>

#include "type_defs.hh"

namespace Maps {

using std::unique_ptr, std::make_unique;
using std::optional, std::nullopt;

unsigned int Type::arity() const {
    if (const auto function_type = std::get_if<std::unique_ptr<FunctionTypeComplex>>(&complex))
        return (*function_type)->arity();
    return 0;
}

bool Type::is_complex() const {
    return !std::holds_alternative<std::monostate>(complex);
}

bool Type::is_function() const {
    return std::holds_alternative<unique_ptr<FunctionTypeComplex>>(complex);
}

Type::Type(ID id, TypeTemplate* type_template): type_template(type_template), id(id) {
    complex = std::monostate{};
}

// copy constructor
Type::Type(const Type& rhs) {
    *this = rhs;
}

// todo: memoize this on the type
std::string Type::to_string() const {
    if (!is_function())
        return static_cast<std::string>(name());

    return function_type()->to_string();
}

std::string FunctionTypeComplex::to_string() const {
    // TODO: check pureness here
    if (arity() == 0) {
        return "Void -> " + return_type->to_string();
    }

    std::string output = "";

    for (const Type* arg: arg_types) {
        output += arg->to_string();
        output += " -> ";
    }

    output += return_type->to_string();

    return output;
}

Type& Type::operator=(const Type& rhs) {
    if (this == &rhs)
        return *this;

    type_template = rhs.type_template;

    switch (rhs.complex.index()) {
        case 0:
            complex = std::monostate{};
            break;

        case 1: // function type
            complex = std::make_unique<FunctionTypeComplex>(*std::get<std::unique_ptr<FunctionTypeComplex>>(rhs.complex));
            break;
    }

    return *this;
}

bool operator==(const Type& lhs, const Type& rhs) {
    if (lhs.complex.index() != rhs.complex.index())
        return false;

    if (lhs.name() != rhs.name())
        return false;
        
    if (!lhs.is_complex())
        return true; // both are simple

    if (std::holds_alternative<std::unique_ptr<FunctionTypeComplex>>(lhs.complex)) {
        auto& lhs_complex = *(std::get<std::unique_ptr<FunctionTypeComplex>>(lhs.complex));
        auto& rhs_complex = *(std::get<std::unique_ptr<FunctionTypeComplex>>(rhs.complex));
        // both are functiontypes
        return lhs_complex == rhs_complex;
    }

    assert(false && "unhandled alternative in Type::==");
    return false;
}

TypeConstructor::TypeConstructor(const std::string& name, int arity)
:name_(name), arity_(arity) {
}


// builtin types are inserted so that they can be looked up nicely
TypeRegistry::TypeRegistry() {
    next_id_ = BUILTIN_TYPES.size();

    for (auto type: BUILTIN_TYPES) {
        types_by_identifier_.insert({static_cast<std::string>(type->name()), type});
        types_by_id_.push_back(type);
    }

    for (auto type_constructor: TypeConstructors::BUILTINS) {
        typeconstructors_by_identifier.insert({type_constructor->name_, type_constructor});
    }
}

optional<const Type*> TypeRegistry::get(const std::string& identifier) {
    auto it = types_by_identifier_.find(identifier);
    if (it == types_by_identifier_.end())
        return nullopt;

    return it->second;
}

const Type* TypeRegistry::get_function_type(const Type& return_type, const std::vector<const Type*>& arg_types, 
    bool is_pure) {

    Type::HashableSignature signature = make_function_signature(return_type, arg_types, is_pure);
    auto existing_it = types_by_structure_.find(signature);

    // if type with this structure exists, just return that
    if (existing_it != types_by_structure_.end())
        return existing_it->second;
    
    // else create that type
    return create_function_type(signature, return_type, arg_types, is_pure);
}

const Type* TypeRegistry::create_opaque_alias(std::string name, const Type* type) {
    assert(false && "not implemented");
}

const Type* TypeRegistry::create_transparent_alias(std::string name, const Type* type) {
    assert(false && "not implemented");
}

Type::HashableSignature TypeRegistry::make_function_signature(const Type& return_type, const std::vector<const Type*>& arg_types, 
    bool is_pure) const {

    // // nullary pure function is just a value
    if (arg_types.size() == 0 && is_pure)
        return std::to_string(return_type.id);

    // // nullary impure function gets the special type =>return_type
    if (arg_types.size() == 0)
        return "=>" + std::to_string(return_type.id);
         

    std::string signature = "";
    bool first = true;
    for (auto arg_type: arg_types) {
        if (!first)
            signature += "-";
        first = false;
        signature += std::to_string(arg_type->id);
    }

    signature += is_pure ? "-" : "=";

    return signature + std::to_string(return_type.id);
}

// NOTE: This actually doesn't work. The type structure notation idea is extremely ambiguous...
// we need to do this by pattern matching instead, but it is what it is
// TODO: mark purity
const Type* TypeRegistry::create_function_type(const Type::HashableSignature& signature, const Type& return_type,
    const std::vector<const Type*>& arg_types, bool is_pure) {

    assert(types_by_id_.size() == next_id_ && "TypeRegistry types_by_id_ not in sync with id:s");

    auto type = make_unique<Type>(get_id(), &Function_);
    type->complex = make_unique<FunctionTypeComplex>(&return_type, arg_types, false);
    auto raw_ptr = type.get();

    types_.push_back(std::move(type));
    types_by_structure_.insert({signature, raw_ptr});
    types_by_id_.push_back(raw_ptr);

    return raw_ptr;
}

} // namespace Maps