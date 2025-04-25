#include "type_inference.hh"

namespace Maps {

// first attempt
bool infer_types(AST& ast) {
    for (auto [_1, callable]: ast.globals_->identifiers_in_order_) {

    }

    return true;
}

} // namespace Maps
