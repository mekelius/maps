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

    bool identifier_exists(const std::string& name) const {
        return identifiers_.find(name) != identifiers_.end();
    }

    std::optional<DefinitionHeader*> get_identifier(const std::string& name) const {
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
    std::map<std::string, DefinitionHeader*> identifiers_;
};

using Scopes = std::span<Scope* const>;
using const_Scopes = std::span<const Scope* const>;

} // namespace Maps

#endif