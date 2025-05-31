#ifndef __FUNCTION_STORE_HH
#define __FUNCTION_STORE_HH

#include <memory>
#include <optional>
#include <string>
#include <map>

#include "llvm/IR/DerivedTypes.h"

#include "mapsc/types/type.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/ast/definition.hh"

namespace Maps {
    
class Type; 
class FunctionType; 

namespace LLVM_IR {

class FunctionStore {
public:
    static constexpr std::string get_suffix(const Maps::FunctionType& type) {
        std::string pureness_marker = type.is_pure() ? "" : "_i";
        std::string suffix = pureness_marker + "_" + type.return_type()->function_signature_string();

        // append arg names
        for (const Maps::Type* param_type: type.param_types())
            suffix += "_" + param_type->function_signature_string();

        return suffix;
    }

    static constexpr std::string get_suffix(const Maps::DefinitionHeader& definition) {
        auto type = definition.get_type();

        if (!type->is_function())
            return ((type->is_pure() ? "" : "_i") + std::string{"_"} + type->function_signature_string());

        return get_suffix(*dynamic_cast<const Maps::FunctionType*>(type));
    }

    // std::optional<llvm::Function*> get_function(const std::string& name, AST::Type* function_type) const;
    std::optional<llvm::FunctionCallee> get(const Maps::DefinitionHeader& definition) const;

    bool insert(const std::string& name, llvm::FunctionCallee function_callee);
    bool insert(const Maps::DefinitionHeader&, llvm::FunctionCallee function_callee);

    bool insert_overloaded(const std::string& name, const Maps::FunctionType& type, 
        llvm::FunctionCallee function_callee);
    bool insert_overloaded(const Maps::DefinitionHeader& definition, llvm::FunctionCallee function_callee);

    std::map<std::string, llvm::FunctionCallee> functions_{};
};

} // namespace LLVM_IR
} // nameespace Maps

#endif