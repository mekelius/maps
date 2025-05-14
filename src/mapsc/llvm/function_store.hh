#ifndef __FUNCTION_STORE_HH
#define __FUNCTION_STORE_HH

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "llvm/IR/DerivedTypes.h"

#include "mapsc/types/type.hh"

namespace Maps { class FunctionType; }


namespace IR {

class FunctionStore {
    using Signature = Maps::Type::HashableSignature;

public:
    // std::optional<llvm::Function*> get_function(const std::string& name, AST::Type* function_type) const;
    std::optional<llvm::FunctionCallee> get(const std::string& name, 
        const Maps::FunctionType& ast_type) const;
    bool insert(const std::string& name, const Maps::FunctionType& ast_type, 
        llvm::FunctionCallee function_callee);

private:
    using InnerMapType = std::unordered_map<Signature, llvm::FunctionCallee>;

    std::unordered_map<std::string, std::unique_ptr<InnerMapType>>
        functions_ = std::unordered_map<std::string, std::unique_ptr<InnerMapType>>();
};

} // namespace IR

#endif