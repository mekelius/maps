#include "layer2_expression.hh"

#include "mapsc/ast/ast_store.hh"

namespace Maps {

std::vector<Expression*>& Expression::terms() {
    return std::get<TermedExpressionValue>(value).terms;
}

const std::vector<Expression*>& Expression::terms() const {
    return std::get<TermedExpressionValue>(value).terms;
}

Scope* Expression::termed_context() const {
    assert(expression_type == ExpressionType::layer2_expression && 
        "Expression::termed_context called on a non-termed expression");

    return std::get<TermedExpressionValue>(value).context;
}


void Expression::mark_not_type_declaration() {
    if (expression_type != ExpressionType::layer2_expression)
        return;

    std::get<TermedExpressionValue>(value).is_type_declaration = DeferredBool::false_;
}

DeferredBool Expression::is_type_declaration() {
    switch (expression_type) {
        case ExpressionType::layer2_expression:
            return std::get<TermedExpressionValue>(value).is_type_declaration;

        case ExpressionType::type_identifier:
        case ExpressionType::type_construct:
        case ExpressionType::type_reference:
        case ExpressionType::type_constructor_reference:
            return DeferredBool::true_;

        default:
            return DeferredBool::false_;
    }
}

Expression* create_layer2_expression(AST_Store& store, std::vector<Expression*>&& terms, 
    Scope* context, const SourceLocation& location) {
    
    return store.allocate_expression({ExpressionType::layer2_expression, 
        TermedExpressionValue{terms, context}, &Hole, location});
}

Expression* create_layer2_expression_testing(AST_Store& store, std::vector<Expression*>&& terms, 
    const SourceLocation& location, bool top_level) {

    assert(false && "not fixed");

    // auto context = store.allocate_definition(DefinitionBody{"testing_definition", Undefined{}, 
    //     top_level, NO_SOURCE_LOCATION});
    
    // return store.allocate_expression({ExpressionType::layer2_expression, 
    //     TermedExpressionValue{terms, context}, &Hole, location});
}


} // namespace Maps
