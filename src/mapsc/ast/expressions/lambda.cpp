#include "../expression.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

using std::nullopt, std::optional;

namespace Maps {

// LambdaExpressionValue LambdaExpressionValue::const_value(AST_Store& ast_store, Expression* value, 
//     const std::vector<const Type*>& param_types, const SourceLocation& location) {

//     ParameterList params{};

//     for (auto param_type: param_types)
//         params.push_back(RT_Definition::discarded_parameter(ast_store, param_type, location));

//     return LambdaExpressionValue{params, nullopt, DefinitionBody{value}};
// }

// LambdaExpressionValue& Expression::lambda_value() {
//     assert(std::holds_alternative<LambdaExpressionValue>(value));
//     return std::get<LambdaExpressionValue>(value);
// }

// const LambdaExpressionValue& Expression::lambda_value() const {
//     assert(std::holds_alternative<LambdaExpressionValue>(value));
//     return std::get<LambdaExpressionValue>(value);
// }

// Expression* Expression::lambda(CompilationState& state, const LambdaExpressionValue& value, 
//     const Type* return_type, bool is_pure, const SourceLocation& location) {

//     auto [parameter_list, scope, body] = value;

//     std::vector<const Type*> parameter_types{};

//     for (auto parameter: parameter_list)
//         parameter_types.push_back(parameter->get_type());

//     auto type = state.types_->get_function_type(return_type, parameter_types, is_pure);
//     return state.ast_store_->allocate_expression(
//         {ExpressionType::lambda, value, type, location});
// }

// Expression* Expression::lambda(CompilationState& state, const LambdaExpressionValue& value, 
//     bool is_pure, const SourceLocation& location) {

//     return lambda(state, value, &Hole, is_pure, location);
// }

std::tuple<Expression*, RT_Definition*> Expression::const_lambda(CompilationState& state, 
    Expression* value, const std::vector<const Type*>& param_types, const SourceLocation& location) {

    auto& ast_store = *state.ast_store_;

    ParameterList parameter_list{};
    auto inner_scope = ast_store.allocate_scope(RT_Scope{});

    for (auto param_type: param_types)
        parameter_list.push_back(RT_Definition::discarded_parameter(ast_store, param_type, location));

    auto definition = RT_Definition::function_definition(state, parameter_list, inner_scope, value,
        false, location);

    definition->set_type(state.types_->get_function_type(value->type, param_types, true));
    auto expression = Expression::reference(ast_store, definition, location);

    return {expression, definition};
}

std::tuple<Expression*, RT_Definition*> Expression::const_lambda(CompilationState& state, 
    KnownValue value, const std::vector<const Type*>& param_types, const SourceLocation& location) {

    auto value_expr = Expression::known_value(state, value, location);
    return Expression::const_lambda(state, value_expr, param_types, location);
}

} // namespace Maps
