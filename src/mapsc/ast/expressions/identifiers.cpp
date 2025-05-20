#include "../expression.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

namespace Maps {

bool Expression::is_identifier() const {
    switch (expression_type) {
        case ExpressionType::identifier:
        case ExpressionType::type_identifier:
        case ExpressionType::operator_identifier:
        case ExpressionType::type_operator_identifier:
            return true;

        default:
            return false;
    }
}

Expression* Expression::identifier(CompilationState& state, const std::string& value, 
    SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression(
        {ExpressionType::identifier, value, &Hole, location});
    state.unresolved_identifiers_.push_back(expression);
    return expression;
}

Expression* Expression::type_identifier(CompilationState& state, 
    const std::string& value, SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression(
        {ExpressionType::type_identifier, value, &Hole, location});
    state.unresolved_identifiers_.push_back(expression);
    return expression;
}

Expression* Expression::operator_identifier(CompilationState& state, const std::string& value, 
    SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression(
        {ExpressionType::operator_identifier, value, &Hole, location});
    state.unresolved_identifiers_.push_back(expression);
    return expression;
}

Expression* Expression::type_operator_identifier(
    CompilationState& state, const std::string& value, SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression({
        ExpressionType::type_operator_identifier, value, &Void, location});
    state.unresolved_type_identifiers_.push_back(expression);
    return expression;
}
    
} // namespace Maps