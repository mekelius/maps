#include "ast_store.hh"

#include <cassert>

namespace Maps {

bool AST_Store::empty() const {
    return size() == 0;
}

size_t AST_Store::size() const {
    return callables_.size() + expressions_.size() + statements_.size();
}

void AST_Store::delete_expression(Expression* expression) {
    expression->expression_type = ExpressionType::deleted;
}
void AST_Store::delete_expression_recursive(Expression* expression) {
    assert(false && "not implemented");
}

void AST_Store::delete_statement(Statement* statement) {
    statement->statement_type = StatementType::deleted;
}
void AST_Store::delete_statement_recursive(Statement* statement) {
    assert(false && "not implemented");
}

Expression* AST_Store::allocate_expression(const Expression&& expression) {        
    expressions_.push_back(std::make_unique<Expression>(expression));
    return expressions_.back().get();
}

Statement* AST_Store::allocate_statement(const Statement&& statement) {
    statements_.push_back(std::make_unique<Statement>(statement));
    return statements_.back().get();
}

RT_Callable* AST_Store::allocate_callable(const RT_Callable&& callable) {
    callables_.push_back(std::make_unique<RT_Callable>(callable));
    return callables_.back().get();
}

RT_Callable* AST_Store::allocate_operator(const RT_Operator&& op) {
    callables_.push_back(std::make_unique<RT_Operator>(op));
    return callables_.back().get();
}

} // namespace AST