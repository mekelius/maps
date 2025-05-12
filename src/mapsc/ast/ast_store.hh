#ifndef __AST_HH
#define __AST_HH

#include <string>
#include <memory>
#include <optional>

#include <cassert>

#include "mapsc/types/type.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/types/type_defs.hh"

#include "mapsc/ast/statement.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/scope.hh"

namespace Maps {

class AST_Store {
public:
    AST_Store() = default;
    
    bool empty() const;
    size_t size() const;

    // ----- WALKING TREE NODE BY NODE -----

    // ----- ITERATING THROUGH CALLABLES -----
    
    void delete_expression(Expression* expression);
    void delete_expression_recursive(Expression* expression);

    void delete_statement(Statement* statement);
    void delete_statement_recursive(Statement* statement);

    Expression* allocate_expression(const Expression&& expr);
    Statement* allocate_statement(const Statement&& statement);
    Callable* allocate_callable(const Callable&& callable);
    Callable* allocate_operator(const Operator&& op);

private:
    friend Scope; // scope is allowed to call create_expression directly to create call expressions

    // currently these guys, once created, stay in memory forever
    // we could create a way to sweep them by having some sort of "alive"-flag
    // or maybe "DeletedStatement" statement type
    // probably won't need that until we do the interpreter
    // TODO: move from vector of unique_ptrs to unique_ptr of vectors
    std::vector<std::unique_ptr<Statement>> statements_ = {};
    std::vector<std::unique_ptr<Expression>> expressions_ = {};
    std::vector<std::unique_ptr<Callable>> callables_ = {};
};

} // namespace Maps

#endif