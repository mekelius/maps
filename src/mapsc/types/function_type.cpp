#include "function_type.hh"

#include "mapsc/types/casts.hh"

namespace Maps {

FunctionType::FunctionType(const ID id, const TypeTemplate* type_template, const Type* return_type, 
    const std::vector<const Type*>& arg_types, bool is_pure)
    :Type(id, type_template, not_castable, not_concretizable), return_type_(return_type), param_types_(arg_types), is_pure_(is_pure) {
}

std::string FunctionType::to_string() const {
    // TODO: check pureness here
    if (arity() == 0) {
        return "Void -> " + return_type_->to_string();
    }

    std::string output = "";

    for (const Type* arg: param_types_) {
        output += arg->to_string();
        output += " -> ";
    }

    output += return_type_->to_string();

    return output;
}

} // namespace Maps