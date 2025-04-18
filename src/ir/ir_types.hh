#ifndef __IR_TYPES_HH
#define __IR_TYPES_HH

#include <optional>

#include "llvm/IR/Type.h"
#include "llvm/IR/LLVMContext.h"

#include "../lang/ast.hh"

namespace IR {

class TypeMap {
  public:
    TypeMap(llvm::LLVMContext& context);

    std::unordered_map<std::string_view, llvm::Type*> type_map_;

    llvm::Type* char_t;
    llvm::Type* int_t;
    llvm::Type* char_array_13_t;
    llvm::Type* char_array_ptr_t;
    llvm::Type* double_t;
    llvm::Type* void_t;

    std::optional<llvm::Type*> convert_type(const AST::Type*) const;
};

}

#endif