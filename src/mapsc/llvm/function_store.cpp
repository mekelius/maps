#include "function_store.hh"

#include <utility>
#include <vector>
#include <variant>

#include "llvm/IR/DerivedTypes.h"

#include "mapsc/logging.hh"
#include "mapsc/types/function_type.hh"

using std::optional, std::nullopt, std::vector, std::tuple, std::get, std::get_if, std::unique_ptr, std::make_unique;

using Maps::LogInContext, Maps::LogContext;

namespace IR {

std::optional<llvm::FunctionCallee> FunctionStore::get(const std::string& name, 
    const Maps::FunctionType& function_type, bool log_error_on_fail) const {
    
    auto outer_it = functions_.find(name);

    if (outer_it == functions_.end())
        return nullopt;

    auto inner_map = outer_it->second.get();
    auto inner_it = inner_map->find(function_type.to_string());

    if (inner_it == inner_map->end()) {
        if (log_error_on_fail)
            LogInContext<LogContext::ir_gen>::compiler_error(
                "function \"" + name + "\" has not been specialized for type \"" + 
                function_type.to_string() + "\"", NO_SOURCE_LOCATION);
        return nullopt;
    }

    return inner_it->second;
}

bool FunctionStore::insert(const std::string& name, const Maps::FunctionType& maps_type, 
    llvm::FunctionCallee function_callee) {    
    
    auto signature = maps_type.to_string();

    auto outer_it = functions_.find(name);

    if (outer_it == functions_.end()) {
        functions_.insert({name, make_unique<InnerMapType>()});
        functions_.at(name)->insert({signature, function_callee});
        return true;
    }

    auto inner_map = outer_it->second.get();
    if (inner_map->find(signature) != inner_map->end()) {
        LogInContext<LogContext::ir_gen_init>::compiler_error(
            "tried to insert a function overload that already exists", 
            COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    inner_map->insert({signature, function_callee});
    return true;
}

} // namepsace IR