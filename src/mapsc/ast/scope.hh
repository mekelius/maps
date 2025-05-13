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
template <typename Ptr_t>
class BaseScope {
public:
    using const_iterator = std::vector<std::pair<std::string, Ptr_t>>::const_iterator;
    const_iterator begin() const { return identifiers_in_order_.begin(); }
    const_iterator end() const { return identifiers_in_order_.end(); }

    BaseScope() = default;

    // std::optional<Expression*> create_call_expression(
    //     const std::string& callee_name, std::vector<Expression*> args, SourceLocation location /*, expected return type?*/);
    // Expression* create_call_expression(Callable* callee, const std::vector<Expression*>& args, 
    //     SourceLocation location /*, expected return type?*/);

    bool identifier_exists(const std::string& name) const {
        return identifiers_.find(name) != identifiers_.end();
    }

    std::optional<Ptr_t> get_identifier(const std::string& name) const {
        auto it = identifiers_.find(name);
        if (it == identifiers_.end())
            return {};

        return it->second;
    }

    std::optional<Ptr_t> create_identifier(Ptr_t callable) {
        auto name = std::string{callable->name_};

        if (identifier_exists(name))
            return std::nullopt;

        identifiers_.insert(std::pair<std::string, Ptr_t>{name, callable});
        identifiers_in_order_.push_back({name, callable});
        
        return callable;
    }

    std::vector<std::pair<std::string, Ptr_t>> identifiers_in_order_ = {};

private:
    std::unordered_map<std::string, Ptr_t> identifiers_;
};

using Scope = BaseScope<Callable* const>;
using ConstScope = BaseScope<const Callable* const>;

} // namespace AST

#endif