#include "../expression.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

namespace Maps {

bool Expression::is_constant_value() const {
    switch (expression_type) {
        case ExpressionType::value:
        case ExpressionType::numeric_literal:
        case ExpressionType::string_literal:
            return true;

        default:
            return false;   
    }
}

bool Expression::is_literal() const {
    switch (expression_type) {
        case ExpressionType::numeric_literal:
        case ExpressionType::string_literal:
            return true;

        default:
            return false;
    }
}

Expression* Expression::string_literal(AST_Store& store, const std::string& value, 
    SourceLocation location) {
    
    return store.allocate_expression({ExpressionType::string_literal, value, &String, location});
}

Expression* Expression::numeric_literal(AST_Store& store, const std::string& value, 
    SourceLocation location) {
    
    return store.allocate_expression(
        {ExpressionType::numeric_literal, value, &NumberLiteral, location});
}

} // namespace Maps
