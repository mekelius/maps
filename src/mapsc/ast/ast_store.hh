#ifndef __AST_HH
#define __AST_HH

#include <cstddef>
#include <memory>
#include <vector>

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/statement.hh"

namespace Maps {

class AST_Store {
public:
    AST_Store() = default;
    
    bool empty() const;
    size_t size() const;

    void delete_expression(Expression* expression);
    void delete_expression_recursive(Expression* expression);

    void delete_statement(Statement* statement);
    void delete_statement_recursive(Statement* statement);

    Expression* allocate_expression(const Expression&& expr);
    Statement* allocate_statement(const Statement&& statement);
    RT_Definition* allocate_definition(const RT_Definition&& definition);
    RT_Definition* allocate_operator(const RT_Operator&& op);
    RT_Scope* allocate_scope(const RT_Scope&& scope);

private:

    // currently these guys, once created, stay in memory forever
    // we could create a way to sweep them by having some sort of "alive"-flag
    // or maybe "DeletedStatement" statement type
    // probably won't need that until we do the interpreter
    // TODO: move from vector of unique_ptrs to unique_ptr of vectors
    std::vector<std::unique_ptr<Statement>> statements_ = {};
    std::vector<std::unique_ptr<Expression>> expressions_ = {};
    std::vector<std::unique_ptr<RT_Definition>> definitions_ = {};
    std::vector<std::unique_ptr<RT_Scope>> scopes_ = {};
};

} // namespace Maps

#endif