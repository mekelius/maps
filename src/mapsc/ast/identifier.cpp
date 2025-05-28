#include "identifier.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/ast/expression.hh"

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

Expression* create_identifier(AST_Store& store, RT_Scope* scope, const std::string& value, 
    const SourceLocation& location) {
    
    Expression* expression = store.allocate_expression(
        {ExpressionType::identifier, value, &Hole, location});
    return expression;
}

Expression* create_operator_identifier(AST_Store& store, RT_Scope* scope, 
    const std::string& value, const SourceLocation& location) {
    
    Expression* expression = store.allocate_expression(
        {ExpressionType::operator_identifier, value, &Hole, location});
    return expression;
}

Expression* create_type_identifier(AST_Store& store, const std::string& value, 
    const SourceLocation& location) {
    
    Expression* expression = store.allocate_expression(
        {ExpressionType::type_identifier, value, &Hole, location});
    return expression;
}

Expression* create_type_operator_identifier(AST_Store& store, const std::string& value, 
    const SourceLocation& location) {
    
    Expression* expression = store.allocate_expression({
        ExpressionType::type_operator_identifier, value, &Void, location});
    return expression;
}
    
} // namespace Maps