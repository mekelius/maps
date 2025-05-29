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

std::string Statement::log_message_string() const {
    switch (statement_type) {
        case StatementType::deleted:
            return "deleted statement";
        case StatementType::compiler_error:
            return "compiler error";
        case StatementType::user_error:
            return "broken statement";
        case StatementType::empty:
            return "empty statement";
        case StatementType::expression_statement:
            return std::get<Expression*>(value)->log_message_string();

        case StatementType::block:
            return "block";

        case StatementType::assignment:
            return "assignment to " + std::get<Assignment>(value).identifier_or_reference->log_message_string();

        case StatementType::return_:
            return "return statement";

        case StatementType::guard: return "guard statement";
        case StatementType::if_chain: return "if-else statement";
        case StatementType::switch_s: return "switch statement";
        case StatementType::loop: return "loop";
    }
}

bool Statement::is_illegal_as_single_statement_block() const {
    switch (statement_type) {
        case StatementType::expression_statement:
        case StatementType::return_:
        case StatementType::user_error:
        case StatementType::compiler_error:
        case StatementType::empty:
        case StatementType::block:
        case StatementType::switch_s:
        case StatementType::if_chain:
        case StatementType::guard:
        case StatementType::loop:
            return false;

        case StatementType::assignment:
            // ???
            return true;

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
        Statement{StatementType::empty, Undefined{}, location});
}
Statement* create_assignment_statement(AST_Store& ast_store, Expression* identifier_or_reference, RT_Definition* definition, const SourceLocation& location) {
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
        Statement{StatementType::user_error, Undefined{}, location});
}
Statement* create_compiler_error_statement(AST_Store& ast_store, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::compiler_error, Undefined{}, location});
}
Statement* create_if(AST_Store& ast_store, Expression* condition, Statement* body, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::if_chain, IfChainValue{{IfBranch{condition, body}}}, location});
}
Statement* create_if_else_chain(AST_Store& ast_store, const IfChain& chain, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::if_chain, IfChainValue{chain}, location});
}
Statement* create_if_else_chain(AST_Store& ast_store, const IfChain& chain, Statement* final_else, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::if_chain, IfChainValue{chain, final_else}, location});
}
Statement* create_if_else_chain(AST_Store& ast_store, const IfChain& chain, optional<Statement*> final_else, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::if_chain, IfChainValue{chain, final_else}, location});
}
Statement* create_guard(AST_Store& ast_store, Expression* condition, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::guard, condition, location});
}
Statement* create_switch(AST_Store& ast_store, Expression* key, const std::vector<CaseBlock>& cases, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::switch_s, SwitchStatementValue{key, cases}, location});
}
Statement* create_while(AST_Store& ast_store, Expression* condition, Block block, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::loop, LoopStatementValue{condition, block}, location});
}
Statement* create_for(AST_Store& ast_store, Statement* initializer, Expression* condition, const Block& block, const SourceLocation& location) {
    return ast_store.allocate_statement(
        Statement{StatementType::loop, LoopStatementValue{condition, block, initializer}, location});
}

} // namespace AST