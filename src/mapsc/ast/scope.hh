#ifndef __SCOPE_HH
#define __SCOPE_HH

#include <map>
#include <optional>
#include <variant>
#include <vector>
#include <cassert>
#include <algorithm>
#include <span>

#include "mapsc/ast/definition.hh"
#include "mapsc/source_location.hh"
#include "mapsc/logging.hh"
#include "mapsc/types/type.hh"

namespace Maps {

class DefinitionHeader;

using Log_resolution = LogInContext<LogContext::name_resolution>;
using Log_creation = LogInContext<LogContext::definition_creation>;

// Scopes contain names bound to definitions
class Scope {
public:
    using const_iterator = std::vector<DefinitionHeader*>::const_iterator;

    const_iterator begin() const { return identifiers_in_order_.begin(); }
    const_iterator end() const { return identifiers_in_order_.end(); }
    bool empty() const { return identifiers_.empty(); }
    size_t size() const { return identifiers_.size(); }

    Scope() = default;
    Scope(Scope* parent_scope):parent_scope_(parent_scope) {}
    Scope(const Scope& other) = default;
    Scope& operator=(const Scope& other) = default;
    ~Scope() = default;

    bool identifier_exists(std::string_view name) const {
        return identifiers_.find(name) != identifiers_.end();
    }

    std::optional<DefinitionHeader*> get_identifier(std::string_view name) const {
        auto it = identifiers_.find(name);
        if (it == identifiers_.end())
            return std::nullopt;

        return it->second;
    }

    std::optional<DefinitionHeader*> create_identifier(DefinitionHeader* node);

    std::vector<DefinitionHeader*> identifiers_in_order_ = {};

    bool is_top_level_scope() { return !parent_scope().has_value(); }
    std::optional<Scope*> parent_scope() { return parent_scope_; } 
    
private:
    std::optional<Scope*> parent_scope_ = std::nullopt;
    std::map<std::string, DefinitionHeader*, std::less<>> identifiers_;
};

// Scopes contain names bound to definitions
template <size_t size_p>
class BuiltinScope {
public:
    using const_iterator = std::array<const DefinitionHeader*, size_p>::const_iterator;

    struct Compare {
        constexpr bool operator() (auto lhs, auto rhs) const { 
            return lhs->name_ < rhs->name_; 
        }

        constexpr bool operator() (auto lhs, std::string_view rhs) const { 
            return lhs->name_ < rhs; 
        }

        constexpr bool operator() (std::string_view lhs, auto rhs) const { 
            return lhs < rhs->name_; 
        }
    };

    const_iterator begin() const { return identifiers_.begin(); }
    const_iterator end() const { return identifiers_.end(); }
    consteval size_t size() const { return size_p; }

    constexpr BuiltinScope(auto... identifiers)
    :identifiers_{identifiers...} {
        // std::copy(identifiers.begin(), identifiers.end(), identifiers_.begin());
        std::sort(identifiers_.begin(), identifiers_.end(), Compare{});
    }

    // BuiltinScope(const BuiltinScope& other) = default;
    // BuiltinScope& operator=(const BuiltinScope& other) = default;

    constexpr bool identifier_exists(std::string_view name) const {
        return std::binary_search(identifiers_.begin(), identifiers_.end(), name, Compare{});
    }

    constexpr std::optional<const DefinitionHeader*> get_identifier(std::string_view name) const {
        auto it = std::lower_bound(identifiers_.begin(), identifiers_.end(), name, Compare{});
        if (it == identifiers_.end() || (*it)->name_ != name)
            return std::nullopt;

        return *it;
    }

private:
    std::array<const DefinitionHeader*, size_p> identifiers_;
};

template<typename...Args>
BuiltinScope(Args&&...) -> BuiltinScope<sizeof...(Args)>;

using Scopes = std::span<Scope* const>;
using const_Scopes = std::span<const Scope* const>;

} // namespace Maps

#endif