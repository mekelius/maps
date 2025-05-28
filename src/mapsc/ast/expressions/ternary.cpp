#include "../expression.hh"

namespace Maps {

TernaryExpressionValue& Expression::ternary_value() {
    assert(std::holds_alternative<TernaryExpressionValue>(value));
    return std::get<TernaryExpressionValue>(value);
}

const TernaryExpressionValue& Expression::ternary_value() const {
    assert(std::holds_alternative<TernaryExpressionValue>(value));
    return std::get<TernaryExpressionValue>(value);
}

} // namespace Maps
