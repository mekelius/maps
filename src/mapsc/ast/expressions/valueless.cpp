#include "../expression.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

namespace Maps {

// valueless expression types are tie, empty, syntax_error and not_implemented
Expression* Expression::valueless(AST_Store& store, ExpressionType expression_type, 
    SourceLocation location) {
    
    return store.allocate_expression({expression_type, std::monostate{}, &Absurd, location});
}

Expression* Expression::minus_sign(AST_Store& store, SourceLocation location) {
    return store.allocate_expression(
        Expression{ExpressionType::minus_sign, std::monostate{}, location});
}

Expression* Expression::user_error(AST_Store& store, SourceLocation location) {
    return store.allocate_expression(Expression{ExpressionType::user_error, location});
}

Expression* Expression::compiler_error(AST_Store& store, SourceLocation location) {
    return store.allocate_expression(Expression{ExpressionType::compiler_error, location});
}

} // namespace Maps
