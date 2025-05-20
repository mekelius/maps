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
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/operator.hh"

namespace Maps {

class CT_Definition;

/**
 * Scopes contain names bound to definitions
 * TODO: implement a constexpr hashmap that can be initialized in static memory 
 */
template <typename T>
class Scope_T {
public:
    using const_iterator = std::vector<std::pair<std::string, T>>::const_iterator;

    const_iterator begin() const { return identifiers_in_order_.begin(); }
    const_iterator end() const { return identifiers_in_order_.end(); }
    bool empty() const { return identifiers_.empty(); }
    size_t size() const { return identifiers_.size(); }

    Scope_T() = default;
    Scope_T(const Scope_T& other) = default;
    Scope_T& operator=(const Scope_T& other) = default;
    ~Scope_T() = default;

    bool identifier_exists(const std::string& name) const {
        return identifiers_.find(name) != identifiers_.end();
    }

    std::optional<T> get_identifier(const std::string& name) const {
        auto it = identifiers_.find(name);
        if (it == identifiers_.end())
            return std::nullopt;

        return it->second;
    }

    std::optional<T> create_identifier(const std::string& name, T definition) {
        if (identifier_exists(name))
            return std::nullopt;

        identifiers_.insert(std::pair<std::string, T>{name, definition});
        identifiers_in_order_.push_back({name, definition});
        
        return definition;
    }

    std::optional<T> create_identifier(T definition) {
        return create_identifier(definition->to_string(), definition);
    }

    std::vector<std::pair<std::string, T>> identifiers_in_order_ = {};

private:
    std::unordered_map<std::string, T> identifiers_;
};

using Scope = Scope_T<Definition*>;
using RT_Scope = Scope_T<RT_Definition*>;
using CT_Scope = Scope_T<CT_Definition*>;

} // namespace AST

#endif