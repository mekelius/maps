#ifndef __STATEMENT_HH
#define __STATEMENT_HH

#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "mapsc/source.hh"
#include "mapsc/ast/definition.hh"

namespace Maps {

struct Expression;
struct Statement;
class AST_Store;

// ----- STATEMENTS -----

struct Assignment {
    std::string identifier; 
    DefinitionBody body;

    bool operator==(const Assignment& other) const = default;
};

using Block = std::vector<Statement*>;

enum class StatementType {
    user_error,                 // parsing failed
    compiler_error,
    deleted,
    empty,
    expression_statement,   // statement consisting of a single expression
    block,
    assignment,
    return_,
    // if,
    // else,
    // for,
    // for_in,
    // do_while,
    // do_for,
    // while/until,
    // switch,
};

using StatementValue = std::variant<
    std::string,
    Expression*,

    Assignment,
    Block,
    Undefined
>;

struct Statement {
    static Statement* empty(AST_Store& store, SourceLocation location);
    static Statement* return_(AST_Store& store, Expression* value, SourceLocation location);
    static Statement* block(AST_Store& store, Block value, SourceLocation location);
    static Statement* expression(AST_Store& store, Expression* value, SourceLocation location);
    static Statement* syntax_error(AST_Store& store, SourceLocation location);

    Statement(StatementType statement_type, SourceLocation location);

    StatementType statement_type;
    SourceLocation location;
    StatementValue value;

    std::string log_message_string() const;

    bool is_illegal_as_single_statement_block() const;
    bool is_empty() const;
    bool is_definition() const;

    bool operator==(const Statement& other) const {
        return std::tie(statement_type, value) == std::tie(other.statement_type, other.value);
    };
};

} // namespace Maps

#endif