#include "identifier.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/expression.hh"

namespace Maps {

Expression* create_identifier(AST_Store& store, Scope* scope, const std::string& value, 
    const SourceLocation& location) {
    
    Expression* expression = store.allocate_expression(
        {ExpressionType::identifier, value, &Unknown, location});
    return expression;
}

Expression* create_operator_identifier(AST_Store& store, Scope* scope, 
    const std::string& value, const SourceLocation& location) {
    
    Expression* expression = store.allocate_expression(
        {ExpressionType::operator_identifier, value, &Unknown, location});
    return expression;
}

Expression* create_type_identifier(AST_Store& store, const std::string& value, 
    const SourceLocation& location) {
    
    Expression* expression = store.allocate_expression(
        {ExpressionType::type_identifier, value, &Unknown, location});
    return expression;
}

Expression* create_type_operator_identifier(AST_Store& store, const std::string& value, 
    const SourceLocation& location) {
    
    Expression* expression = store.allocate_expression({
        ExpressionType::type_operator_identifier, value, &Void, location});
    return expression;
}
    
} // namespace Maps