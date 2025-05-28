#include "termed_expression.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

namespace Maps {

std::vector<Expression*>& Expression::terms() {
    return std::get<TermedExpressionValue>(value).terms;
}

const std::vector<Expression*>& Expression::terms() const {
    return std::get<TermedExpressionValue>(value).terms;
}

RT_Definition* Expression::termed_context() const {
    assert(expression_type == ExpressionType::termed_expression && 
        "Expression::termed_context called on a non-termed expression");

    return std::get<TermedExpressionValue>(value).context;
}


void Expression::mark_not_type_declaration() {
    if (expression_type != ExpressionType::termed_expression)
        return;

    std::get<TermedExpressionValue>(value).is_type_declaration = DeferredBool::false_;
}

DeferredBool Expression::is_type_declaration() {
    switch (expression_type) {
        case ExpressionType::termed_expression:
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

Expression* create_termed(AST_Store& store, std::vector<Expression*>&& terms, 
    RT_Definition* context, const SourceLocation& location) {
    
    return store.allocate_expression({ExpressionType::termed_expression, 
        TermedExpressionValue{terms, context}, &Hole, location});
}

Expression* create_termed_testing(AST_Store& store, std::vector<Expression*>&& terms, 
    const SourceLocation& location, bool top_level) {

    auto context = store.allocate_definition(RT_Definition{"testing_definition", Undefined{}, 
        top_level, NO_SOURCE_LOCATION});
    
    return store.allocate_expression({ExpressionType::termed_expression, 
        TermedExpressionValue{terms, context}, &Hole, location});
}


} // namespace Maps
