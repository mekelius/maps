#include "../expression.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

namespace Maps {

LambdaExpressionValue& Expression::lambda_value() {
    assert(std::holds_alternative<LambdaExpressionValue>(value));
    return std::get<LambdaExpressionValue>(value);
}

const LambdaExpressionValue& Expression::lambda_value() const {
    assert(std::holds_alternative<LambdaExpressionValue>(value));
    return std::get<LambdaExpressionValue>(value);
}

} // namespace Maps
