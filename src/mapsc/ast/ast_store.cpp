#include "ast_store.hh"

#include <cassert>

#include "mapsc/logging.hh"

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

DefinitionHeader* AST_Store::allocate_definition_header(RT_DefinitionHeader definition) {
    using Log = LogInContext<LogContext::definition_creation>;

    definition_headers_.push_back(std::make_unique<RT_DefinitionHeader>(std::move(definition)));
    Log::debug_extra(definition.location()) << "Allocated definition header " << definition << Endl;
    auto allocated_header = definition_headers_.back().get();

    // allocated_header->name_ = dynamic_cast<RT_DefinitionHeader*>(allocated_header)->name_string_;

    return allocated_header;
}

std::pair<DefinitionHeader*, DefinitionBody*> AST_Store::allocate_definition(RT_DefinitionHeader header, 
    const LetDefinitionValue& body) {
    
    auto allocated_header = allocate_definition_header(std::move(header));
    auto allocated_body = allocate_definition_body(allocated_header, body);

    return {allocated_header, allocated_body};
}

DefinitionBody* AST_Store::allocate_definition_body(DefinitionHeader* header, const LetDefinitionValue& body) {
    using Log = LogInContext<LogContext::definition_creation>;

    definition_bodies_.push_back(std::make_unique<DefinitionBody>(header, body));
    auto allocated_body = definition_bodies_.back().get();

    allocated_body->header_ = header;
    header->body_ = allocated_body;

    Log::debug_extra(header->location()) << "Allocated definition body " << *header << Endl;

    return allocated_body;
}

Operator* AST_Store::allocate_operator(const Operator&& definition) {
    definition_headers_.push_back(std::make_unique<Operator>(definition));
    return dynamic_cast<Operator*>(definition_headers_.back().get());
}

Parameter* AST_Store::allocate_parameter(const Parameter&& definition) {
    definition_headers_.push_back(std::make_unique<Parameter>(definition));
    return dynamic_cast<Parameter*>(definition_headers_.back().get());
}

External* AST_Store::allocate_external(const External&& definition) {
    definition_headers_.push_back(std::make_unique<External>(definition));
    return dynamic_cast<External*>(definition_headers_.back().get());
}

Scope* AST_Store::allocate_scope(const Scope&& scope) {
    scopes_.push_back(std::make_unique<Scope>(scope));
    return scopes_.back().get();
}

} // namespace AST