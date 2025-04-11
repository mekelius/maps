#ifndef __AST_HH
#define __AST_HH

#include <vector>
#include <unordered_map>
#include <tuple>
#include <string>
#include <memory>
#include <string_view>
#include <optional>
#include <cassert>
#include <variant>

#include "pragmas.hh"
#include "types.hh"
#include "../logging.hh"

namespace AST {

struct Expression;
struct Statement;

using CallableBody = std::variant<std::monostate, Expression*, Statement*>;

// ----- EXPRESSIONS -----

enum class ExpressionType {
    string_literal          = 0,    // literal: Value value
    numeric_literal         = 1,
    call                    = 2,    // call: Callee identifier, [Expression] args
    deferred_call           = 3,    // call where the callee is an expression
    native_function         = 4,
    native_operator         = 5,
    termed_expression       = 6,    // basically something that layer1 can't handle
    tie                     = 7,    // lack of whitespace between an operator and another term
    unresolved_identifier   = 8,    // something to be hoisted
    unresolved_operator     = 9,
    syntax_error            = 10,   // reached something that shouldn't have been such as trying to parse eof as expression
    not_implemented         = 11,
    deleted                 = 12,
};

using CallExpressionValue = std::tuple<std::string, std::vector<Expression*>>;
using CallExpressionValueDeferred = std::tuple<Expression*, std::vector<Expression*>>;
using TermedExpressionValue = std::vector<Expression*>;

using ExpressionValue = std::variant<
    std::monostate,
    std::string, 
    CallExpressionValue, 
    CallExpressionValueDeferred, 
    TermedExpressionValue
>;

struct Expression {
    // TODO: move initializing expression values from AST::create_expression
    Expression(ExpressionType expr_type, SourceLocation location, const Type* type): 
    expression_type(expr_type), location(location), type(type) {};
    
    ExpressionType expression_type;
    SourceLocation location;
    const Type* type = &Hole;
    ExpressionValue value;

    TermedExpressionValue& terms() {
        return std::get<TermedExpressionValue>(value);
    }
    CallExpressionValue& call() {
        return std::get<CallExpressionValue>(value);
    }
    std::string& string_value() {
        return std::get<std::string>(value);
    }
};

// ----- STATEMENTS -----
struct Let{
    std::string identifier; 
    CallableBody body;
};

struct Assignment{
    std::string identifier; 
    CallableBody body;
};

using Block = std::vector<Statement*>;

enum class StatementType {
    broken                  , // parsing failed
    illegal                 , // well formed but illegal statements
    empty                   ,
    expression_statement    , // statement consisting of a single expression
    block                   ,
    let                     ,
    assignment              ,
    return_                 ,
    // if
    // else
    // for
    // for_id
    // do_while
    // do_for
    // while/until
    // switch
};

using StatementValue = std::variant<
    std::monostate,
    std::string,
    Expression*,

    Let,
    Assignment,
    Block
>;

struct Statement {
    Statement(StatementType statement_type, SourceLocation location);
    
    StatementType statement_type;
    SourceLocation location;
    StatementValue value;
};


// ----- CALLABLES -----


// struct Let {
//     std::string name;
//     CallableBody body = not_initialized;
// };

// struct Assignment {
//     std::string name;
//     CallableBody body;
// };

struct Callable {
    Callable(const std::string& name, CallableBody body, const Type* return_type, std::vector<const Type*>& arg_types)
    : name(name), body(body), return_type(return_type), arg_types(arg_types) {}

    std::string name;
    CallableBody body;
    const Type* return_type;
    std::vector<const Type*> arg_types;
};

// TODO: move housekeeping here from AST
class Scope {
  public:
    std::unordered_map<std::string, Callable*> identifiers;
    std::vector<std::string> identifiers_in_order = {};
  private:
};

// TODO: expand mind enough for contexts
// context where statements can exist
// class Context {
//   public:
//     enum Type {
//         local,
//         global,
//         global_eval, // global context where evaluation is allowed 
//     };

//     Context::Type context_type = local;
//     std::optional<Scope> scope = std::nullopt;
// };

class AST {
  public:
    Expression* create_expression(
        ExpressionType expression_type, SourceLocation location, const Type* type = &Void);
    Statement* create_statement(StatementType statement_type, SourceLocation location);
    
    // returns nullopt if the name is taken
    std::optional<Callable*> create_callable(
        const std::string& name,
        CallableBody body,
        const Type* return_type = &Hole,
        std::vector<const Type*> arg_types = {}
    );
    // appends a statement to root_
    void append_top_level_statement(Statement* statement);
    
    // TODO: these should be Scope's responsibility
    bool identifier_exists(const std::string& name) const;
    std::optional<Callable*> get_identifier(const std::string& name);

    bool init_builtin_callables();
    
    // container for top-level statements
    std::vector<Statement*> root_ = {};
    // std::unique_ptr<Context> root_ = std::make_unique<Context>(Context::Type::global);
    Pragmas pragmas;
    bool valid = true;
    Scope global_ = {};

    // layer1 fills these with pointers to expressions that need work so that layer 2 doesn't 
    // need to walk the tree to find them
    std::vector<Expression*> unresolved_identifiers_and_operators = {};
    std::vector<Expression*> unparsed_termed_expressions = {};

  private:
    // caller is responsible for checking if the name is free with name_free
    void create_identifier(const std::string& name, Callable* callable);

    // currently these guys, once created, stay in memory forever
    // we could create a way to sweep them by having some sort of "alive"-flag
    // or maybe "DeletedStatement" statement type
    // probably won't need that until we do the interpreter
    std::vector<std::unique_ptr<Statement>> statements_ = {};
    std::vector<std::unique_ptr<Expression>> expressions_ = {};

    std::unordered_map<std::string, Callable*> identifiers_ = {};
    std::vector<std::unique_ptr<Callable>> callables_ = {};
};

} // namespace AST

#endif