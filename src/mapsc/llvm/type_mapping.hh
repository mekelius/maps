#ifndef __IR_TYPES_HH
#define __IR_TYPES_HH

#include <optional>
#include <span>
#include <string>
#include <unordered_map>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

#include "mapsc/types/type.hh"

namespace llvm { class LLVMContext; }
namespace Maps { class FunctionType; }


namespace IR {

class TypeMap {
public:
    TypeMap(llvm::LLVMContext& context);

    llvm::Type* char_t;
    llvm::Type* int_t;
    llvm::Type* uint_t;
    llvm::Type* memuint_t;
    llvm::PointerType* char_array_ptr_t;
    llvm::Type* double_t;
    llvm::Type* void_t;
    llvm::Type* boolean_t;
    llvm::FunctionType* repl_wrapper_signature;
    llvm::FunctionType* cmain_signature;
    llvm::StructType* mutstring_t;
    llvm::PointerType* mutstring_ptr_t;

    // size_t size() const;
    // bool empty() const;
    bool contains(const Maps::Type& maps_type) const;
    [[nodiscard]] bool insert(const Maps::Type* maps_type, llvm::Type* llvm_type);

    std::optional<llvm::Type*> convert_type(const Maps::Type& type) const;
    std::optional<llvm::FunctionType*> convert_function_type(
        const Maps::Type& return_type, std::span<const Maps::Type* const> arg_types) const;
    std::optional<llvm::FunctionType*> convert_function_type(const Maps::FunctionType& type) const;

    bool is_good_ = true;
    
private:
    std::unordered_map<std::string, llvm::Type*> type_map_;
};

} // namespace IR

#endif