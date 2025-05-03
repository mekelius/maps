#ifndef __SCOPE_HH
#define __SCOPE_HH

#include <unordered_map>
#include <optional>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/types/type.hh"
#include "mapsc/ast/ast_node.hh"

namespace Maps {

class AST_Store;

/**
 * Scopes contain names bound to callables
 * Note that it is the AST that owns the callables, but they can be created through the scope
 */
class Scope {
public:
    using const_iterator = std::vector<std::pair<std::string, Callable*>>::const_iterator;
    const_iterator begin() const { return identifiers_in_order_.begin(); }
    const_iterator end() const { return identifiers_in_order_.end(); }

    Scope(AST_Store* ast): ast_(ast) {};

    bool identifier_exists(const std::string& name) const;
    std::optional<Callable*> get_identifier(const std::string& name) const;

    std::optional<Callable*> create_callable(const std::string& name, CallableBody body, 
        std::optional<SourceLocation> location = std::nullopt);
    std::optional<Callable*> create_callable(const std::string& name, SourceLocation location);

    std::optional<Callable*> create_binary_operator(const std::string& name, CallableBody body,
        Precedence precedence, Associativity associativity, SourceLocation location);

    std::optional<Callable*> create_unary_operator(const std::string& name, CallableBody body,
        UnaryFixity fixity, SourceLocation location);

    std::optional<Expression*> create_reference_expression(const std::string& name, SourceLocation location);
    Expression* create_reference_expression(Callable* callable, SourceLocation location);

    std::optional<Expression*> create_call_expression(
        const std::string& callee_name, std::vector<Expression*> args, SourceLocation location /*, expected return type?*/);
    Expression* create_call_expression(Callable* callee, std::vector<Expression*> args, 
        SourceLocation location /*, expected return type?*/);

    std::vector<std::pair<std::string, Callable*>> identifiers_in_order_ = {};

private:
    friend AST_Store;

    std::unordered_map<std::string, Callable*> identifiers_;
    AST_Store* ast_;
};

} // namespace AST

#endif