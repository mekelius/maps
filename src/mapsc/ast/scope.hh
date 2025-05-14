#ifndef __SCOPE_HH
#define __SCOPE_HH

#include <unordered_map>
#include <optional>
#include <variant>
#include <vector>
#include <cassert>

#include "mapsc/source.hh"
#include "mapsc/types/type.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/callable.hh"
#include "mapsc/ast/operator.hh"

namespace Maps {

/**
 * Scopes contain names bound to callables
 * TODO: implement a constexpr hashmap that can be initialized in static memory 
 */
class Scope {
public:
    using const_iterator = std::vector<std::pair<std::string, Callable*>>::const_iterator;
    const_iterator begin() const { return identifiers_in_order_.begin(); }
    const_iterator end() const { return identifiers_in_order_.end(); }

    Scope() = default;

    // std::optional<Expression*> create_call_expression(
    //     const std::string& callee_name, std::vector<Expression*> args, SourceLocation location /*, expected return type?*/);
    // Expression* create_call_expression(Callable* callee, const std::vector<Expression*>& args, 
    //     SourceLocation location /*, expected return type?*/);

    bool identifier_exists(const std::string& name) const {
        return identifiers_.find(name) != identifiers_.end();
    }

    std::optional<Callable*> get_identifier(const std::string& name) const {
        auto it = identifiers_.find(name);
        if (it == identifiers_.end())
            return std::nullopt;

        return it->second;
    }

    std::optional<Callable*> create_identifier(Callable* callable) {
        auto name = std::string{callable->name_};

        if (identifier_exists(name))
            return std::nullopt;

        identifiers_.insert(std::pair<std::string, Callable*>{name, callable});
        identifiers_in_order_.push_back({name, callable});
        
        return callable;
    }

    std::vector<std::pair<std::string, Callable*>> identifiers_in_order_ = {};

private:
    std::unordered_map<std::string, Callable*> identifiers_;
};

} // namespace AST

#endif