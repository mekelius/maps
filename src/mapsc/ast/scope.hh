#ifndef __SCOPE_HH
#define __SCOPE_HH

#include <map>
#include <optional>
#include <variant>
#include <vector>
#include <cassert>
#include <span>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"
#include "mapsc/types/type.hh"

namespace Maps {

class Definition;
class RT_Definition;
class CT_Definition;

using Log_resolution = LogInContext<LogContext::name_resolution>;
using Log_creation = LogInContext<LogContext::identifier_creation>;

class CT_Definition;

// Scopes contain names bound to definitions
template <typename T>
class Scope_T {
public:
    using const_iterator = std::vector<T>::const_iterator;

    const_iterator begin() const { return identifiers_in_order_.begin(); }
    const_iterator end() const { return identifiers_in_order_.end(); }
    bool empty() const { return identifiers_.empty(); }
    size_t size() const { return identifiers_.size(); }

    Scope_T() = default;
    Scope_T(Scope_T* parent_scope):parent_scope_(parent_scope) {}
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

    std::optional<T> create_identifier(T definition) {
        auto name = definition->name_string();
        if (identifier_exists(name)) {
            Log_creation::debug("Attempting to redefine identifier " + name, definition->location());
            return std::nullopt;
        }

        identifiers_.insert(std::pair<std::string, T>{name, definition});
        identifiers_in_order_.push_back(definition);
        
        Log_creation::debug_extra("Created identifier " + name, definition->location());

        return definition;
    }

    std::vector<T> identifiers_in_order_ = {};

    bool is_top_level_scope() { return !parent_scope().has_value(); }
    std::optional<Scope_T*> parent_scope() { return parent_scope_; } 
    
private:
    std::optional<Scope_T*> parent_scope_ = std::nullopt;
    std::map<std::string, T> identifiers_;
};

using RT_Scope = Scope_T<RT_Definition*>;
using CT_Scope = Scope_T<CT_Definition*>;

using Scopes = std::span<RT_Scope* const>;
using const_Scopes = std::span<const RT_Scope* const>;

} // namespace AST

#endif