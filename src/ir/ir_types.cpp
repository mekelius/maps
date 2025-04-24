#include "ir_types.hh"

using std::optional, std::nullopt, std::vector;

namespace IR {

// TODO: store these somewhere
TypeMap::TypeMap(llvm::LLVMContext& context) {
    // get some types
    char_t = llvm::Type::getInt8Ty(context);
    char_array_13_t = llvm::ArrayType::get(char_t, 13);
    char_array_ptr_t = 
        llvm::PointerType::getUnqual(llvm::PointerType::getUnqual(char_t));
    int_t = llvm::Type::getInt64Ty(context);
    double_t = llvm::Type::getDoubleTy(context);
    void_t = llvm::Type::getVoidTy(context);
    boolean_t = llvm::Type::getInt1Ty(context);

    repl_wrapper_signature = llvm::FunctionType::get(void_t, false);

    cmain_signature = llvm::FunctionType::get(int_t, {int_t, char_array_ptr_t}, false);
}

std::optional<llvm::Type*> TypeMap::convert_type(const Maps::Type& type) const {
    // auto it = type_map_.find(type->name);
    // if (it == type_map_.end())
    //     return std::nullopt;

    // return it->second;
    return {};
}

std::optional<llvm::FunctionType*> TypeMap::convert_function_type(const Maps::Type& return_type, const std::vector<const Maps::Type*>& arg_types) const {
    optional<llvm::Type*> llvm_return_type = convert_type(return_type);
    
    if(!llvm_return_type)
        return nullopt;
    
    vector<llvm::Type*> llvm_arg_types{};

    for (auto arg_type: arg_types) {
        optional<llvm::Type*> llvm_arg_type = convert_type(*arg_type);
        if (!llvm_arg_type)
            return nullopt;

        llvm_arg_types.push_back(*llvm_arg_type);
    }

    return llvm::FunctionType::get(*llvm_return_type, llvm_arg_types, false);
}

std::optional<llvm::FunctionType*> TypeMap::convert_function_type(const Maps::Type& type) const {
    if (!type.is_function())
        return nullopt;

    return convert_function_type(*type.function_type()->return_type, type.function_type()->arg_types);
}

} // namespace IR
