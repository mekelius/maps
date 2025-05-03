#include "function_store.hh"

#include "mapsc/logging.hh"

using Logging::log_error, Logging::log_info;
using std::optional, std::nullopt, std::vector, std::tuple, std::get, std::get_if, std::unique_ptr, std::make_unique;

namespace IR {

std::optional<llvm::FunctionCallee> FunctionStore::get(const std::string& name, 
    const Maps::FunctionType& function_type) const {
    
    auto outer_it = functions_.find(name);

    if (outer_it == functions_.end())
        return nullopt;

    auto inner_map = outer_it->second.get();
    auto inner_it = inner_map->find(function_type.hashable_signature());

    if (inner_it == inner_map->end()) {
        log_error("function \"" + name + "\" has not been specialized for type \"" + 
            function_type.to_string() + "\"");
        return nullopt;
    }

    return inner_it->second;
}

bool FunctionStore::insert(const std::string& name, const Maps::FunctionType& ast_type, 
    llvm::FunctionCallee function_callee) {    
    
    auto signature = ast_type.hashable_signature();

    auto outer_it = functions_.find(name);

    if (outer_it == functions_.end()) {
        functions_.insert({name, make_unique<InnerMapType>()});
        functions_.at(name)->insert({signature, function_callee});
        return true;
    }

    auto inner_map = outer_it->second.get();
    if (inner_map->find(signature) != inner_map->end()) {
        log_error("tried to insert a function overload that already exists");
        return false;
    }

    inner_map->insert({signature, function_callee});
    return true;
}

} // namepsace IR