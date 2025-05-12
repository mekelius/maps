#include "function_type.hh"

#include "mapsc/types/casts.hh"

namespace Maps {

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