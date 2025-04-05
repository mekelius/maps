#include "ast.hh"

namespace AST {

Expression* AST::create_expression(ExpressionType type) {
    expressions_.push_back(std::make_unique<Expression>(type));
    Expression* expression = expressions_.back().get();

    switch (type) {
        case ExpressionType::string_literal:
            // new(&expression->string_value) std::string("");
            return expression;

        case ExpressionType::call:
            expression->call_expr = {"", {}};
            return expression;    
    }
}

} // namespace AST