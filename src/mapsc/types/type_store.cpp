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
            {make_function_signature(type->return_type(), type->param_types(), type->is_pure()), type});
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

// NOTE: This actually doesn't work. The type structure notation idea is extremely ambiguous...
// we need to do this by pattern matching instead, but it is what it is


} // namespace Maps