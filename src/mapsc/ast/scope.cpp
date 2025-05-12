#include "scope.hh"

#include "mapsc/ast/ast_store.hh"

namespace Maps {

bool Scope::identifier_exists(const std::string& name) const {
    return identifiers_.find(name) != identifiers_.end();
}

std::optional<Callable*> Scope::get_identifier(const std::string& name) const {
    auto it = identifiers_.find(name);
    if (it == identifiers_.end())
        return {};

    return it->second;
}

std::optional<Callable*> Scope::create_identifier(Callable* callable) {
    auto name = callable->name_;

    if (identifier_exists(name))
        return std::nullopt;

    identifiers_.insert({name, callable});
    identifiers_in_order_.push_back({name, callable});
    
    return callable;
}

} // namespace AST