#ifndef __STATEMENT_HH
#define __STATEMENT_HH

#include <cassert>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "mapsc/log_format.hh"
#include "mapsc/source_location.hh"
#include "mapsc/ast/definition.hh"

namespace Maps {

struct Expression;
struct Statement;
class AST_Store;

// ----- STATEMENTS -----

enum class StatementType {
    user_error,                 // parsing failed
    compiler_error,
    deleted,
    empty,
    expression_statement,   // statement consisting of a single expression
    block,
    assignment,
    return_,
    guard,
    switch_s,
    conditional,
    loop,
};

constexpr std::string_view statement_type_string(StatementType statement_type) {
    std::string str = std::string{statement_prefix};

    switch (statement_type) {
        case StatementType::user_error: 
            str += "user_error"; break;
        case StatementType::compiler_error: 
            str += "compiler_error"; break;
        case StatementType::deleted: 
            str += "deleted"; break;
        case StatementType::empty: 
            str += "empty"; break;
        case StatementType::expression_statement: 
            str += "expression_statement"; break;
        case StatementType::block: 
            str += "block"; break;
        case StatementType::assignment: 
            str += "assignment"; break;
        case StatementType::return_: 
            str += "return_"; break;
        case StatementType::guard: 
            str += "guard"; break;
        case StatementType::conditional: 
            str += "if-statement"; break;
        case StatementType::switch_s: 
            str += "switch"; break;
        case StatementType::loop: 
            str += "for"; break;
    }

    return std::string_view{str};
}

using Block = std::vector<Statement*>;
using CaseBlock = std::pair<Expression*, Statement*>;
using GuardValue = Expression*;

struct ConditionalValue {
    Expression* condition; 
    Statement* body;
    std::optional<Statement*> else_branch = std::nullopt;

    bool operator==(const ConditionalValue&) const = default;
};

struct SwitchStatementValue {
    Expression* key;
    std::vector<CaseBlock> cases;

    bool operator==(const SwitchStatementValue&) const = default;
};

struct LoopStatementValue {
    Expression* condition;
    Statement* body;
    std::optional<Statement*> initializer = std::nullopt;

    bool operator==(const LoopStatementValue&) const = default;
};

struct Assignment {
    Expression* identifier_or_reference; 
    DefinitionBody* body;

    bool operator==(const Assignment&) const = default;
};

using EmptyStatementValue = std::monostate;

using StatementValue = std::variant<
    std::string,
    Expression*,
    Assignment,
    Block,
    EmptyStatementValue,
    ConditionalValue,
    LoopStatementValue,
    SwitchStatementValue
>;

struct Statement {
    // Statement(StatementType statement_type, const SourceLocation& location);
    Statement(StatementType statement_type, const StatementValue& value, const SourceLocation& location);

    StatementType statement_type;
    SourceLocation location;
    StatementValue value;

    LogStream::InnerStream& log_self_to(LogStream::InnerStream& logstream) const;

    bool is_illegal_as_single_statement_block() const;
    bool is_empty() const;
    bool is_definition() const;

    template <typename T>
    const T& get_value() const { 
        assert(std::holds_alternative<T>(value) && "Statement::get_value called with wrong value type");
        return std::get<T>(value); 
    }

    template <typename T>
    T& get_value() { 
        assert(std::holds_alternative<T>(value) && "Statement::get_value called with wrong value type");
        return std::get<T>(value); 
    }

    constexpr std::string_view statement_type_string() const {
        return Maps::statement_type_string(statement_type);
    };

    bool operator==(const Statement& other) const {
        return std::tie(statement_type, value) == std::tie(other.statement_type, other.value);
    };
};

Statement* create_empty_statement(AST_Store& store, const SourceLocation& location);
Statement* create_assignment_statement(AST_Store& store, Expression* identifier_or_reference, DefinitionBody* definition, const SourceLocation& location);
Statement* create_return_statement(AST_Store& store, Expression* expression, const SourceLocation& location);
Statement* create_block(AST_Store& store, const Block& block, const SourceLocation& location);
Statement* create_expression_statement(AST_Store& store, Expression* expression, const SourceLocation& location);
Statement* create_user_error_statement(AST_Store& store, const SourceLocation& location);
Statement* create_compiler_error_statement(AST_Store& store, const SourceLocation& location);
Statement* create_if(AST_Store& store, Expression* condition, Statement* body, const SourceLocation& location);
Statement* create_if_else(AST_Store& store, Expression* condition, Statement* body, Statement* statement, const SourceLocation& location);
Statement* create_guard(AST_Store& store, Expression* condition, const SourceLocation& location);
Statement* create_switch(AST_Store& store, Expression* key, const CaseBlock& cases, const SourceLocation& location);
Statement* create_while(AST_Store& store, Expression* condition, Statement* body, const SourceLocation& location);
Statement* create_while_else(AST_Store& ast_store, Expression* condition, Statement* body, Statement* else_branch, const SourceLocation& location);
Statement* create_for(AST_Store& store, Statement* initializer, Expression* condition, Statement* body, const SourceLocation& location);

} // namespace Maps

#endif