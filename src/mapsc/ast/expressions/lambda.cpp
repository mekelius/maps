#include "../expression.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

using std::nullopt, std::optional;

namespace Maps {

LambdaExpressionValue& Expression::lambda_value() {
    assert(std::holds_alternative<LambdaExpressionValue>(value));
    return std::get<LambdaExpressionValue>(value);
}

const LambdaExpressionValue& Expression::lambda_value() const {
    assert(std::holds_alternative<LambdaExpressionValue>(value));
    return std::get<LambdaExpressionValue>(value);
}

Expression* Expression::lambda(CompilationState& state, const LambdaExpressionValue& value, 
    const Type* return_type, bool is_pure, SourceLocation location) {

    auto [parameter_list, scope, body] = value;

    std::vector<const Type*> parameter_types{};

    for (auto parameter: parameter_list)
        parameter_types.push_back(parameter->get_type());

    auto type = state.types_->get_function_type(*return_type, parameter_types, is_pure);
    return state.ast_store_->allocate_expression(
        {ExpressionType::lambda, value, type, location});
}

Expression* Expression::lambda(CompilationState& state, const LambdaExpressionValue& value, 
    bool is_pure, SourceLocation location) {

    return lambda(state, value, &Hole, is_pure, location);
}

} // namespace Maps
