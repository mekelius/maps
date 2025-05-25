#ifndef __FUNCTION_STORE_HH
#define __FUNCTION_STORE_HH

#include <memory>
#include <optional>
#include <string>
#include <map>

#include "llvm/IR/DerivedTypes.h"

#include "mapsc/types/type.hh"

namespace Maps { class FunctionType; }


namespace IR {

class FunctionStore {
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