#include "type_inference.hh"

#include <variant>
#include <cassert>

using std::get, std::get_if;

namespace Maps {

bool handle_callable(AST& ast, Callable& callable) {
    assert(false && "not implemented");
    const Type* type = callable.get_type();

    if (!type->is_native()) {

    }

    if (Expression* const* expression = get_if<Expression*>(&callable.body)) {
        // handle_expression();
    }

    if (Statement* const* expression = get_if<Statement*>(&callable.body)) {

    }

    return false;
}

// first attempt
bool infer_types(AST& ast) {
    assert(false && "not implemented");

    for (auto [_1, callable]: ast.globals_->identifiers_in_order_) {
        if (!handle_callable(ast, *callable)) {

        }
    }

    return true;
}

} // namespace Maps
