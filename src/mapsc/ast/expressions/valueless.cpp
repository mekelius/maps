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

Expression* Expression::syntax_error(AST_Store& store, SourceLocation location) {
    return store.allocate_expression(Expression{ExpressionType::syntax_error, location});
}

} // namespace Maps
