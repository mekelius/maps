#ifndef __IR_TYPES_HH
#define __IR_TYPES_HH

#include <optional>

#include "llvm/IR/DerivedTypes.h"
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
    llvm::Type* boolean_t;
    llvm::FunctionType* repl_wrapper_signature;
    llvm::FunctionType* cmain_signature;

    std::optional<llvm::Type*> convert_type(const Maps::Type& type) const;
    std::optional<llvm::FunctionType*> convert_function_type(const Maps::Type& return_type, 
        const std::vector<const Maps::Type*>& arg_types) const;
    std::optional<llvm::FunctionType*> convert_function_type(const Maps::FunctionType& type) const;
};

}

#endif