#include "statement.hh"

#include <cassert>
#include <optional>

#include "mapsc/log_format.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/ast_store.hh"

using std::optional, std::nullopt;

namespace Maps {

// Statement::Statement(StatementType statement_type, const SourceLocation& location)
// :statement_type(statement_type), location(location), value(Undefined{}) {}

Statement::Statement(StatementType statement_type, const StatementValue& value, 
    const SourceLocation& location)
:statement_type(statement_type), location(location), value(value) {}

LogStream::InnerStream& Statement::log_self_to(LogStream::InnerStream& ostream) const {
    switch (statement_type) {
        case StatementType::deleted:
            return ostream << "deleted statement";
        case StatementType::compiler_error:
            return ostream << "compiler error";
        case StatementType::user_error:
            return ostream << "broken statement";
        case StatementType::empty:
            return ostream << "empty statement";
        case StatementType::expression_statement:
            ostream << "expression statement: ";
            return std::get<Expression*>(value)->log_self_to(ostream);

        case StatementType::block:
            return ostream << "block";

        case StatementType::assignment:
            ostream << "assignment to: ";
            return std::get<Assignment>(value).identifier_or_reference->log_self_to(ostream);

        case StatementType::return_:
            ostream << "return statement: ";
            return std::get<Expression*>(value)->log_self_to(ostream);

        case StatementType::guard: return ostream << "guard statement";
        case StatementType::switch_s: return ostream << "switch statement";
        case StatementType::loop: return ostream << "loop";
        case StatementType::conditional: return ostream << "conditional statement";
    }
}

bool Statement::is_illegal_as_single_statement_block() const {
    switch (statement_type) {
        case StatementType::expression_statement:
        case StatementType::return_:
        case StatementType::empty:
        case StatementType::block:
        case StatementType::switch_s:
        case StatementType::loop:
        case StatementType::conditional:
            return false;
        
        case StatementType::assignment:
            // ???
            return true;
        
        case StatementType::user_error:
        case StatementType::compiler_error:
        case StatementType::guard:
        case StatementType::deleted:
            return true;
    }
}

bool Statement::is_empty() const {
    return statement_type == StatementType::empty;
}

bool Statement::is_definition() const {
    switch (statement_type) {
        case StatementType::assignment:
            return true;

        default:
            return false;
    }
}

Statement* create_empty_statement(AST_Store& ast_store, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::empty, {}, location});
}
Statement* create_assignment_statement(AST_Store& ast_store, Expression* identifier_or_reference, DefinitionBody* definition, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::assignment, identifier_or_reference, location});
}
Statement* create_return_statement(AST_Store& ast_store, Expression* expression, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::return_, expression, location});
}
Statement* create_block(AST_Store& ast_store, const Block& block, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::block, Block{block}, location});
}
Statement* create_expression_statement(AST_Store& ast_store, Expression* expression, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::expression_statement, expression, location});
}
Statement* create_user_error_statement(AST_Store& ast_store, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::user_error, {}, location});
}
Statement* create_compiler_error_statement(AST_Store& ast_store, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::compiler_error, {}, location});
}
Statement* create_if(AST_Store& ast_store, Expression* condition, Statement* body, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::conditional, ConditionalValue{condition, body}, location});
}
Statement* create_if_else(AST_Store& ast_store, Expression* condition, Statement* body, Statement* else_body, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::conditional, ConditionalValue{condition, body, else_body}, location});
}
Statement* create_guard(AST_Store& ast_store, Expression* condition, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::guard, condition, location});
}
Statement* create_switch(AST_Store& ast_store, Expression* key, const std::vector<CaseBlock>& cases, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::switch_s, SwitchStatementValue{key, cases}, location});
}
Statement* create_while(AST_Store& ast_store, Expression* condition, Statement* body, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::loop, LoopStatementValue{condition, body}, location});
}
Statement* create_while_else(AST_Store& ast_store, Expression* condition, Statement* body, Statement* else_branch, const SourceLocation& location) {
    auto loop = ast_store.allocate_statement(Statement{StatementType::loop, LoopStatementValue{condition, body}, location});
    return create_if_else(ast_store, condition, loop, else_branch, location);
}
Statement* create_for(AST_Store& ast_store, Statement* initializer, Expression* condition, Statement* body, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::loop, LoopStatementValue{condition, body, initializer}, location});
}

} // namespace AST