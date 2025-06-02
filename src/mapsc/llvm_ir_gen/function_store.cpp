#include "function_store.hh"

#include <utility>
#include <vector>
#include <variant>

#include "llvm/IR/DerivedTypes.h"

#include "mapsc/ast/definition.hh"
#include "mapsc/logging.hh"
#include "mapsc/types/function_type.hh"

using std::optional, std::nullopt, std::vector, std::tuple, std::get, std::get_if, std::unique_ptr, std::make_unique;

using Maps::LogInContext, Maps::LogContext, Maps::NO_SOURCE_LOCATION, Maps::COMPILER_INIT_SOURCE_LOCATION;

namespace Maps {
namespace LLVM_IR {

std::optional<llvm::FunctionCallee> FunctionStore::get(const DefinitionHeader& definition) const {
    using Log = LogInContext<LogContext::ir_gen>;

    auto name = definition.name_string();

    Log::debug_extra(NO_SOURCE_LOCATION) << "Looking up a function with name \"" << name << "\"" << Endl;
    
    auto it = functions_.find(name);
    if (it != functions_.end()) {
        Log::debug_extra(NO_SOURCE_LOCATION) << "Found function" << Endl;
        return it->second;
    }

    auto overload_it = functions_.find(name + get_suffix(definition));
    if (overload_it != functions_.end()) {
        Log::debug_extra(NO_SOURCE_LOCATION) << "Found overload" << Endl;
        return overload_it->second;
    }

    Log::error(NO_SOURCE_LOCATION) << "No function named \"" << name << "\" overloaded for type " << 
        *definition.get_type() << " in function store" << Endl;
    return nullopt;
}

bool FunctionStore::insert(const std::string& name, llvm::FunctionCallee function_callee) {    
    auto it = functions_.find(name);
    if (it != functions_.end()) {
        LogInContext<LogContext::ir_gen_init>::compiler_error(NO_SOURCE_LOCATION) <<
            "Tried to insert a duplicate function \"" << name << "\" into function store";
        return false;
    }

    functions_.insert({name, function_callee});
    return true;
}

bool FunctionStore::insert(const DefinitionHeader& definition, 
    llvm::FunctionCallee function_callee) {    
    
    if (!definition.get_type()->is_function())
        return false;

    return insert(definition.name_string(), function_callee);
}

bool FunctionStore::insert_overloaded(const std::string& name, const Maps::FunctionType& type, 
    llvm::FunctionCallee function_callee) {    
    
    if (functions_.contains(name)) {
        LogInContext<LogContext::ir_gen_init>::compiler_error(NO_SOURCE_LOCATION) <<
            "Tried to insert a function overload for a existing non-overloaded function";
        return false;
    }

    auto suffixed_name = name + get_suffix(type);
    if (functions_.contains(suffixed_name)) {
        LogInContext<LogContext::ir_gen_init>::compiler_error(NO_SOURCE_LOCATION) <<
            "Tried to insert a duplicate function overload into function store";
        return false;
    }
    
    functions_.insert({suffixed_name, function_callee});
    return true;
}

bool FunctionStore::insert_overloaded(const DefinitionHeader& definition, 
    llvm::FunctionCallee function_callee) {    
    
    if (!definition.get_type()->is_function())
            return false;

    return insert_overloaded(definition.name_string(), 
        *dynamic_cast<const Maps::FunctionType*>(definition.get_type()), function_callee);
}

} // namepsace LLVM_IR
} // nameespace Maps
