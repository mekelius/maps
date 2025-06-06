#ifndef __AST_HH
#define __AST_HH

#include <cstddef>
#include <memory>
#include <vector>

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/external.hh"
#include "mapsc/ast/definition_body.hh"
#include "mapsc/ast/function_definition.hh"
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
    DefinitionHeader* allocate_definition_header(RT_DefinitionHeader definition);
    std::pair<DefinitionHeader*, DefinitionBody*> allocate_definition(
        RT_DefinitionHeader header, const LetDefinitionValue& body);
    DefinitionBody* allocate_definition_body(DefinitionHeader*, const LetDefinitionValue& body);
    Operator* allocate_operator(RT_Operator definition);
    Parameter* allocate_parameter(const Parameter&& definition);
    External* allocate_external(const External&& definition);

    Scope* allocate_scope(const Scope&& scope);

private:

    // currently these guys, once created, stay in memory forever
    // we could create a way to sweep them by having some sort of "alive"-flag
    // or maybe "DeletedStatement" statement type
    // probably won't need that until we do the interpreter
    // TODO: move from vector of unique_ptrs to unique_ptr of vectors
    std::vector<std::unique_ptr<Statement>> statements_ = {};
    std::vector<std::unique_ptr<Expression>> expressions_ = {};
    std::vector<std::unique_ptr<DefinitionHeader>> definition_headers_ = {};
    std::vector<std::unique_ptr<DefinitionBody>> definition_bodies_ = {};
    std::vector<std::unique_ptr<Scope>> scopes_ = {};
};

} // namespace Maps

#endif