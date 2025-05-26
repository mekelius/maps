#include "function_store.hh"

#include <utility>
#include <vector>
#include <variant>

#include "llvm/IR/DerivedTypes.h"

#include "mapsc/ast/definition.hh"
#include "mapsc/logging.hh"
#include "mapsc/types/function_type.hh"

using std::optional, std::nullopt, std::vector, std::tuple, std::get, std::get_if, std::unique_ptr, std::make_unique;

using Maps::LogInContext, Maps::LogContext;

namespace IR {

std::optional<llvm::FunctionCallee> FunctionStore::get(const Maps::Definition& definition) const {
    using Log = LogInContext<LogContext::ir_gen>;

    auto name = definition.name_string();

    Log::debug_extra("Looking up a function with name \"" + name + "\"", NO_SOURCE_LOCATION);
    
    auto it = functions_.find(name);
    if (it != functions_.end()) {
        Log::debug_extra("Found function" , NO_SOURCE_LOCATION);
        return it->second;
    }

    auto overload_it = functions_.find(name + get_suffix(definition));
    if (overload_it != functions_.end()) {
        Log::debug_extra("Found overload" , NO_SOURCE_LOCATION);
        return overload_it->second;
    }

    Log::error("No function named \"" + name + "\" overloaded for type " + 
        definition.get_type()->name_string() + " in function store", NO_SOURCE_LOCATION);
    return nullopt;
}

bool FunctionStore::insert(const std::string& name, llvm::FunctionCallee function_callee) {    
    auto it = functions_.find(name);
    if (it != functions_.end()) {
        LogInContext<LogContext::ir_gen_init>::compiler_error(
            "Tried to insert a duplicate function \"" + name + "\" into function store", 
            COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    functions_.insert({name, function_callee});
    return true;
}

bool FunctionStore::insert(const Maps::Definition& definition, 
    llvm::FunctionCallee function_callee) {    
    
    if (!definition.get_type()->is_function())
        return false;

    return insert(definition.name_string(), function_callee);
}

bool FunctionStore::insert_overloaded(const std::string& name, const Maps::FunctionType& type, 
    llvm::FunctionCallee function_callee) {    
    
    if (functions_.contains(name)) {
        LogInContext<LogContext::ir_gen_init>::compiler_error(
            "Tried to insert a function overload for a existing non-overloaded function", 
            COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    auto suffixed_name = name + get_suffix(type);
    if (functions_.contains(suffixed_name)) {
        LogInContext<LogContext::ir_gen_init>::compiler_error(
            "Tried to insert a duplicate function overload into function store", 
            COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }
    
    functions_.insert({suffixed_name, function_callee});
    return true;
}

bool FunctionStore::insert_overloaded(const Maps::Definition& definition, 
    llvm::FunctionCallee function_callee) {    
    
    if (!definition.get_type()->is_function())
            return false;

    return insert_overloaded(definition.name_string(), 
        *dynamic_cast<const Maps::FunctionType*>(definition.get_type()), function_callee);
}

std::optional<llvm::FunctionCallee> SimpleFunctionStore::get(const std::string& name, 
    bool log_error_on_fail) const {

    using Log = LogInContext<LogContext::ir_gen>;

    Log::debug_extra("Looking up a function with name \"" + name + "\"", NO_SOURCE_LOCATION);
    
    auto it = functions_.find(name);

    if (it == functions_.end()) {
        Log::error("No function named \"" + name + "\" in function store", NO_SOURCE_LOCATION);
        return nullopt;
    }
    Log::debug_extra("Found function" , NO_SOURCE_LOCATION);

    return it->second;
}

bool SimpleFunctionStore::insert(const std::string& name, llvm::FunctionCallee function_callee) {    
    auto it = functions_.find(name);

    if (it != functions_.end()) {
        LogInContext<LogContext::ir_gen_init>::compiler_error(
            "tried to insert a function overload that already exists", 
            COMPILER_INIT_SOURCE_LOCATION);
    }
    
    functions_.insert({name, function_callee});
    return true;
}

std::optional<llvm::FunctionCallee> PolymorphicFunctionStore::get(const std::string& name, 
    const Maps::FunctionType& function_type, bool log_error_on_fail) const {

    using Log = LogInContext<LogContext::ir_gen>;

    Log::debug_extra("Looking up a function with name \"" + name + "\" and type \"" + 
        function_type.name_string() + "\"...", NO_SOURCE_LOCATION);
    
    auto outer_it = functions_.find(name);

    if (outer_it == functions_.end()) {
        Log::error("No function named \"" + name + "\" in function store", NO_SOURCE_LOCATION);
        return nullopt;
    }

    Log::debug_extra("Found name, looking up type..." , NO_SOURCE_LOCATION);

    auto inner_map = outer_it->second.get();
    auto inner_it = inner_map->find(function_type.name_string());

    if (inner_it == inner_map->end()) {
        if (log_error_on_fail)
            LogInContext<LogContext::ir_gen>::compiler_error(
                "Function \"" + name + "\" has not been specialized for type \"" + 
                function_type.name_string() + "\"", NO_SOURCE_LOCATION);
        return nullopt;
    }

    Log::debug_extra("Found function" , NO_SOURCE_LOCATION);

    return inner_it->second;
}

bool PolymorphicFunctionStore::insert(const std::string& name, const Maps::FunctionType& maps_type, 
    llvm::FunctionCallee function_callee) {    
    
    auto signature = maps_type.name_string();

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