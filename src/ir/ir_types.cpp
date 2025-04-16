#include "ir_types.hh"
#include "llvm/IR/DerivedTypes.h"

namespace IR {

TypeMap::TypeMap(llvm::LLVMContext& context) {
    // get some types
    char_t = llvm::Type::getInt8Ty(context);
    char_array_13_t = llvm::ArrayType::get(char_t, 13);
    char_array_ptr_t = 
        llvm::PointerType::getUnqual(llvm::PointerType::getUnqual(char_t));
    int_t = llvm::Type::getInt64Ty(context);
    double_t = llvm::Type::getDoubleTy(context);
    void_t = llvm::Type::getVoidTy(context);
}

std::optional<llvm::Type*> TypeMap::convert_type(const AST::Type* type) const {
    // auto it = type_map_.find(type->name);
    // if (it == type_map_.end())
    //     return std::nullopt;

    // return it->second;
    return {};
}


} // namespace IR
