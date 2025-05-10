#ifndef __STATEMENT_HH
#define __STATEMENT_HH

#include <string>
#include <tuple>
#include <vector>

#include "common/maps_datatypes.h"

#include "mapsc/source.hh"

#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/callable.hh"
#include "mapsc/ast/operator.hh"

namespace Maps {

// ----- BUILTINS -----

class Callable;
struct Expression;
struct Statement;

// ----- STATEMENTS -----
struct Let {
    std::string identifier; 
    CallableBody body;

    bool operator==(const Let& other) const = default;
};

struct OperatorStatementValue {
    std::string op;
    unsigned int arity;
    CallableBody body;
    // include the specifiers
    bool operator==(const OperatorStatementValue& other) const = default;

};

struct Assignment {
    std::string identifier; 
    CallableBody body;

    bool operator==(const Assignment& other) const = default;
};

using Block = std::vector<Statement*>;

enum class StatementType {
    deleted,
    broken,                 // parsing failed
    illegal,                // well formed but illegal statements
    empty,
    expression_statement,   // statement consisting of a single expression
    block,
    let,
    operator_definition,
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
    std::monostate,
    std::string,
    Expression*,

    Let,
    OperatorStatementValue,
    Assignment,
    Block
>;

struct Statement {
    Statement(StatementType statement_type, SourceLocation location);
    
    StatementType statement_type;
    SourceLocation location;
    StatementValue value;

    std::string log_message_string() const;

    bool is_illegal_as_single_statement_block() const;

    bool operator==(const Statement& other) const = default;
};

} // namespace Maps

#endif