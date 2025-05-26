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

namespace Maps { class Type; class FunctionType; class Definition; }

namespace IR {

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

    static constexpr std::string get_suffix(const Maps::Definition& definition) {
        auto type = definition.get_type();

        if (!type->is_function())
            return ((type->is_pure() ? "" : "_i") + std::string{"_"} + type->function_signature_string());

        return get_suffix(*dynamic_cast<const Maps::FunctionType*>(type));
    }

    // std::optional<llvm::Function*> get_function(const std::string& name, AST::Type* function_type) const;
    std::optional<llvm::FunctionCallee> get(const Maps::Definition& definition) const;

    bool insert(const std::string& name, llvm::FunctionCallee function_callee);
    bool insert(const Maps::Definition&, llvm::FunctionCallee function_callee);

    bool insert_overloaded(const std::string& name, const Maps::FunctionType& type, 
        llvm::FunctionCallee function_callee);
    bool insert_overloaded(const Maps::Definition& definition, llvm::FunctionCallee function_callee);

    std::map<std::string, llvm::FunctionCallee> functions_{};
};

class SimpleFunctionStore {
public:
    // std::optional<llvm::Function*> get_function(const std::string& name, AST::Type* function_type) const;
    std::optional<llvm::FunctionCallee> get(const std::string& name, 
        bool log_error_on_fail = true) const;
    bool insert(const std::string& name, llvm::FunctionCallee function_callee);

    std::map<std::string, llvm::FunctionCallee> functions_{};
};

class PolymorphicFunctionStore {
    using Signature = std::string;

public:
    // std::optional<llvm::Function*> get_function(const std::string& name, AST::Type* function_type) const;
    std::optional<llvm::FunctionCallee> get(const std::string& name, 
        const Maps::FunctionType& maps_type, bool log_error_on_fail = true) const;
    bool insert(const std::string& name, const Maps::FunctionType& maps_type, 
        llvm::FunctionCallee function_callee);

// private:
    using InnerMapType = std::map<Signature, llvm::FunctionCallee>;

    std::map<std::string, std::unique_ptr<InnerMapType>>
        functions_ = std::map<std::string, std::unique_ptr<InnerMapType>>();
};

} // namespace IR

#endif