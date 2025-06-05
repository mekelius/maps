#include "misc_expression.hh"

#include "mapsc/ast/ast_store.hh"

namespace Maps {

// valueless expression types are tie, empty, syntax_error and not_implemented
Expression* create_valueless(AST_Store& store, ExpressionType expression_type, 
    const SourceLocation& location) {
    
    return store.allocate_expression({expression_type, std::monostate{}, &Untyped, location});
}

Expression* create_minus_sign(AST_Store& store, const SourceLocation& location) {
    return store.allocate_expression(
        Expression{ExpressionType::minus_sign, std::monostate{}, location});
}

Expression* create_user_error(AST_Store& store, const SourceLocation& location) {
    return store.allocate_expression(Expression{ExpressionType::user_error, location});
}

Expression* create_compiler_error(AST_Store& store, const SourceLocation& location) {
    return store.allocate_expression(Expression{ExpressionType::compiler_error, location});
}

} // namespace Maps
