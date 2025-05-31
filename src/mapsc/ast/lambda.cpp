#include "lambda.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/ast/reference.hh"
#include "mapsc/ast/value.hh"

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

std::tuple<Expression*, DefinitionBody*> create_const_lambda(CompilationState& state, 
    Expression* value, std::span<const Type* const> param_types, 
    const SourceLocation& location, bool is_pure) {

    auto& ast_store = *state.ast_store_;

    ParameterList parameter_list{};
    auto inner_scope = ast_store.allocate_scope(Scope{});

    assert(false && "not updated");

    // for (auto param_type: param_types)
    //     parameter_list.push_back(LetDefinition::discarded_parameter(ast_store, param_type, location));

    // auto definition = LetDefinition::function_definition(state, parameter_list, inner_scope, value,
    //     false, location);

    // definition->set_type(state.types_->get_function_type(value->type, param_types, is_pure));
    // auto expression = create_reference(ast_store, definition, location);

    // return {expression, definition};
}

std::tuple<Expression*, DefinitionBody*> create_const_lambda(CompilationState& state, 
    KnownValue value, std::span<const Type* const> param_types, const SourceLocation& location,
    bool is_pure) {

    auto value_expr = create_known_value(state, value, location);
    return create_const_lambda(state, value_expr, param_types, location, is_pure);
}

} // namespace Maps
