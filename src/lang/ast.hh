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

namespace AST {

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

struct Expression;

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
    Expression(ExpressionType expr_type, const Type* type): expression_type(expr_type), type(type) {};
    
    ExpressionType expression_type;
    const Type* type = &Hole;
    ExpressionValue value;

    TermedExpressionValue& terms() {
        assert(std::holds_alternative<TermedExpressionValue>(value) 
            && "Expression::terms() called on wrong kind of expression");
        return std::get<TermedExpressionValue>(value);
    }
    
    CallExpressionValue& call() {
        assert(std::holds_alternative<CallExpressionValue>(value) 
            && "Expression::terms() called on wrong kind of expression");
        return std::get<CallExpressionValue>(value);
    }
};

// ----- STATEMENTS -----

struct BrokenStatement {};

struct IllegalStatement {
    std::string reason;
};

struct EmptyStatement {
};

struct ExpressionStatement {
    Expression* expression;
};

struct ReturnStatement {
    Expression* expression;
};

struct LetStatement;
struct AssignmentStatement;
struct BlockStatement;

// !important! these need to be in the same order
enum class StatementType {
    broken                  = 0, // parsing failed
    illegal                 = 1, // well formed but illegal statements
    empty                   = 2,
    expression_statement    = 3, // statement consisting of a single expression
    block                   = 4,
    let                     = 5,
    assignment              = 6,
    return_                 = 7,
    // if
    // else
    // for
    // for_id
    // do_while
    // do_for
    // while/until
    // switch
};
using Statement = std::variant<
    BrokenStatement,    // 0
    IllegalStatement,   // 1
    EmptyStatement,     // 2
    ExpressionStatement,// 3
    BlockStatement,     // 4
    LetStatement,       // 5
    AssignmentStatement,// 6
    ReturnStatement     // 7
>;

struct NotInitialized {};
constexpr NotInitialized not_initialized;
using CallableBody = std::variant<NotInitialized, Expression*, Statement*>;

struct LetStatement {
    std::string name;
    CallableBody body = not_initialized;
};

struct AssignmentStatement {
    std::string name;
    CallableBody body;
};

struct BlockStatement {
    std::vector<Statement*> statements;
};

inline StatementType statement_type(Statement* statement) {
    return static_cast<StatementType>(statement->index());
}

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
    Expression* create_expression(ExpressionType expression_type, const Type* type = &Void);
    Statement* create_statement(Statement&& statement);
    
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
    bool name_free(const std::string& name) const;
    std::optional<Callable*> get_identifier(const std::string& name);

    bool init_builtin_callables();
    
    // container for top-level statements
    std::vector<Statement*> root_ = {};
    // std::unique_ptr<Context> root_ = std::make_unique<Context>(Context::Type::global);
    Pragmas pragmas;
    bool valid = true;
    Scope global_ = {};

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