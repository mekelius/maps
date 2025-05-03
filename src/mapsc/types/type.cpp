#include "type.hh"

#include <cassert>

#include "mapsc/logging.hh"
#include "mapsc/ast/ast_node.hh"

#include "mapsc/types/type_defs.hh"
#include "mapsc/types/casts.hh"

using Logging::log_error;

namespace Maps {

using std::unique_ptr, std::make_unique;
using std::optional, std::nullopt;

// todo: memoize this on the type
std::string Type::to_string() const {
    return static_cast<std::string>(name());
}

bool Type::cast_to(const Type* target_type, Expression* expression) const {
    
    if (*expression->type == *target_type)
        return true;

    if (!expression->is_castable_expression()) {
        log_error("expression " + expression->log_message_string() + ", is not castable");
        return false;
    }
    
    return cast_to_(target_type, expression);
}

FunctionType::FunctionType(const ID id, const TypeTemplate* type_template, const Type* return_type, 
    const std::vector<const Type*>& arg_types, bool is_pure)
    :Type(id, type_template, not_castable), return_type_(return_type), arg_types_(arg_types), is_pure_(is_pure) {
}

std::string FunctionType::to_string() const {
    // TODO: check pureness here
    if (arity() == 0) {
        return "Void -> " + return_type_->to_string();
    }

    std::string output = "";

    for (const Type* arg: arg_types_) {
        output += arg->to_string();
        output += " -> ";
    }

    output += return_type_->to_string();

    return output;
}

// builtin types are inserted so that they can be looked up nicely
TypeRegistry::TypeRegistry() {
    next_id_ = BUILTIN_TYPES.size();

    for (auto type: BUILTIN_TYPES) {
        types_by_identifier_.insert({static_cast<std::string>(type->name()), type});
        types_by_id_.push_back(type);
    }

    // insert type constructors
    // for (auto type_constructor: BUILTIN_TYPECONSTRUCTORS) {
    //     typeconstructors_by_identifier.insert({type_constructor->name_, type_constructor});
    // }
}

optional<const Type*> TypeRegistry::get(const std::string& identifier) {
    auto it = types_by_identifier_.find(identifier);
    if (it == types_by_identifier_.end())
        return nullopt;

    return it->second;
}

const FunctionType* TypeRegistry::get_function_type(const Type& return_type, const std::vector<const Type*>& arg_types, 
    bool is_pure) {

    Type::HashableSignature signature = make_function_signature(return_type, arg_types, is_pure);
    auto existing_it = types_by_structure_.find(signature);

    // if type with this structure exists, just return that
    if (existing_it != types_by_structure_.end())
        return dynamic_cast<const FunctionType*>(existing_it->second);
    
    // else create that type
    return create_function_type(signature, return_type, arg_types, is_pure);
}

Type::HashableSignature TypeRegistry::make_function_signature(const Type& return_type, 
    const std::vector<const Type*>& arg_types, bool is_pure) const {

    // // nullary pure function is just a value
    if (arg_types.size() == 0 && is_pure)
        return std::to_string(return_type.id_);

    // // nullary impure function gets the special type =>return_type
    if (arg_types.size() == 0)
        return "=>" + std::to_string(return_type.id_);
         

    std::string signature = "";
    bool first = true;
    for (auto arg_type: arg_types) {
        if (!first)
            signature += "-";
        first = false;
        signature += std::to_string(arg_type->id_);
    }

    signature += is_pure ? "-" : "=";

    return signature + std::to_string(return_type.id_);
}

// NOTE: This actually doesn't work. The type structure notation idea is extremely ambiguous...
// we need to do this by pattern matching instead, but it is what it is
// TODO: mark purity
const FunctionType* TypeRegistry::create_function_type(const Type::HashableSignature& signature, 
    const Type& return_type, const std::vector<const Type*>& arg_types, bool is_pure) {

    assert(types_by_id_.size() == static_cast<size_t>(next_id_) && 
        "TypeRegistry types_by_id_ not in sync with id:s");

    auto type = make_unique<FunctionType>(get_id(), &Function_, &return_type, arg_types, is_pure);
    auto raw_ptr = type.get();

    types_.push_back(std::move(type));
    types_by_structure_.insert({signature, raw_ptr});
    types_by_id_.push_back(raw_ptr);

    return raw_ptr;
}

} // namespace Maps