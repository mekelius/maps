#include "ast_store.hh"

#include <cassert>

namespace Maps {

bool AST_Store::empty() const {
    return size() == 0;
}

size_t AST_Store::size() const {
    return definitions_.size() + expressions_.size() + statements_.size();
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

RT_Definition* AST_Store::allocate_definition(const RT_Definition&& definition) {
    definitions_.push_back(std::make_unique<RT_Definition>(definition));
    return definitions_.back().get();
}

RT_Definition* AST_Store::allocate_operator(const RT_Operator&& op) {
    definitions_.push_back(std::make_unique<RT_Operator>(op));
    return definitions_.back().get();
}

RT_Scope* AST_Store::allocate_scope(const RT_Scope&& scope) {
    scopes_.push_back(std::make_unique<RT_Scope>(scope));
    return scopes_.back().get();
}

} // namespace AST