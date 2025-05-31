#include "ast_store.hh"

#include <cassert>

namespace Maps {

bool AST_Store::empty() const {
    return size() == 0;
}

size_t AST_Store::size() const {
    return definition_headers_.size() + 
        definition_bodies_.size() + 
        expressions_.size() + 
        statements_.size();
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

DefinitionHeader* AST_Store::allocate_definition(const DefinitionHeader&& definition) {
    definition_headers_.push_back(std::make_unique<DefinitionHeader>(definition));
    return definition_headers_.back().get();
}

DefinitionHeader* AST_Store::allocate_definition(const DefinitionHeader&& header, const LetDefinitionValue& body) {
    auto allocated_header = allocate_definition(std::move(header));

    definition_bodies_.push_back(std::make_unique<DefinitionBody>(allocated_header, body));
    auto allocated_body = definition_bodies_.back().get();

    allocated_header->body_ = allocated_body;
    allocated_body->header_ = allocated_header;

    return allocated_header;
}

DefinitionHeader* AST_Store::allocate_definition_body(DefinitionHeader* header, const LetDefinitionValue& body) {
    definition_bodies_.push_back(std::make_unique<DefinitionBody>(header, body));
    auto allocated_body = definition_bodies_.back().get();

    allocated_body->header_ = header;
    header->body_ = allocated_body;

    return header;
}


Scope* AST_Store::allocate_scope(const Scope&& scope) {
    scopes_.push_back(std::make_unique<Scope>(scope));
    return scopes_.back().get();
}

} // namespace AST