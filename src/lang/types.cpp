#include "types.hh"

namespace AST {

unsigned int Type::arity() const {
    if (const auto function_type = std::get_if<FunctionType>(&ct))
        return function_type->arity();
    return 0;
}

bool Type::is_complex() const {
    return !std::holds_alternative<std::monostate>(ct);
}

} // namespace AST