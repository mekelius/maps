#include "type_store.hh"

#include <cassert>
#include <utility>

#include "mapsc/types/function_type.hh"

using std::optional, std::nullopt;

namespace Maps {

// builtin types are inserted so that they can be looked up nicely
TypeStore::TypeStore(const std::span<const Type* const> builtin_simple_types, 
    const std::span<const FunctionType* const> builtin_function_types) {
    
    for (auto type: builtin_simple_types) {
        types_by_identifier_.insert({static_cast<std::string>(type->name()), type});
    }

    for (auto type: builtin_function_types) {
        types_by_structure_.insert(
            {make_function_signature(*type->return_type_, type->get_params()), type});
    }

    // insert type constructors
    // for (auto type_constructor: BUILTIN_TYPECONSTRUCTORS) {
    //     typeconstructors_by_identifier.insert({type_constructor->name_, type_constructor});
    // }
}

bool TypeStore::empty() const {
    return types_.empty();
}

size_t TypeStore::size() const {
    return types_.size();
}

optional<const Type*> TypeStore::get(const std::string& identifier) {
    auto it = types_by_identifier_.find(identifier);
    if (it == types_by_identifier_.end())
        return nullopt;

    return it->second;
}

const FunctionType* TypeStore::get_function_type(const Type& return_type, 
    const std::vector<const Type*>& arg_types, bool is_pure) {

    Type::HashableSignature signature = make_function_signature(return_type, arg_types, is_pure);
    auto existing_it = types_by_structure_.find(signature);

    // if type with this structure exists, just return that
    if (existing_it != types_by_structure_.end())
        return dynamic_cast<const FunctionType*>(existing_it->second);
    
    // else create that type
    return create_function_type(signature, return_type, arg_types, is_pure);
}

Type::HashableSignature TypeStore::make_function_signature(const Type& return_type, 
    const std::span<const Type* const> arg_types, bool is_pure) const {

    // nullary pure function is just a value
    if (arg_types.size() == 0 && is_pure)
        return std::string{return_type.name()};

    // nullary impure function gets the special type =>return_type
    if (arg_types.size() == 0)
        return "=>" + std::string{return_type.name()};

    std::string signature = "";
    bool first = true;
    for (auto arg_type: arg_types) {
        if (!first)
            signature += "-";
        first = false;
        signature += std::string{arg_type->name()};
    }

    signature += is_pure ? "-" : "=";

    return signature + std::string{return_type.name()};
}

// NOTE: This actually doesn't work. The type structure notation idea is extremely ambiguous...
// we need to do this by pattern matching instead, but it is what it is
const FunctionType* TypeStore::create_function_type(const Type::HashableSignature& signature,
    const Type& return_type, const std::vector<const Type*>& arg_types, bool is_pure) {

    std::unique_ptr<const Type> up = 
        make_unique<const RTFunctionType>(&return_type, arg_types, is_pure);
    types_.push_back(std::move(up));
    auto raw_ptr = types_.back().get();

    types_by_structure_.insert({signature, raw_ptr});

    return dynamic_cast<const FunctionType*>(raw_ptr);
}

} // namespace Maps