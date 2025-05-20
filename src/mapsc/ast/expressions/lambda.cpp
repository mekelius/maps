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

Expression* Expression::lambda(AST_Store& store, Expression* binding_type_declaration, 
    DefinitionBody body, SourceLocation location) {

    return store.allocate_expression(
        {ExpressionType::lambda, LambdaExpressionValue{binding_type_declaration, body}, location});
}


} // namespace Maps
