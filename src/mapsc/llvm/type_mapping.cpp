#include "type_mapping.hh"

#include <utility>
#include <vector>

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

#include "mapsc/logging.hh"


#include "mapsc/types/type_defs.hh"
#include "mapsc/types/function_type.hh"

namespace llvm { class LLVMContext; }

using std::optional, std::nullopt, std::vector;

using Log = Maps::LogInContext<Maps::LogContext::ir_gen_init>;

namespace IR {

// TODO: store these somewhere
TypeMap::TypeMap(llvm::LLVMContext& context) {
    // get some types
    char_t = llvm::Type::getInt8Ty(context);
    char_array_ptr_t = 
        llvm::PointerType::getUnqual(llvm::PointerType::getUnqual(char_t));
    int_t = llvm::Type::getInt32Ty(context);
    uint_t = llvm::Type::getInt32Ty(context); // llvm doesn't distinguish based on signedness
    double_t = llvm::Type::getDoubleTy(context);
    void_t = llvm::Type::getVoidTy(context);
    boolean_t = llvm::Type::getInt8Ty(context);
    
    static_assert(sizeof(size_t) == sizeof(int64_t) && "MemUInt needs to be adjusted");
    memuint_t = llvm::Type::getInt64Ty(context);
    
    mutstring_t = llvm::StructType::create(context, {
        char_array_ptr_t, uint_t, memuint_t}, "maps_MutString");
        
    mutstring_ptr_t = llvm::PointerType::getUnqual(mutstring_t);

    repl_wrapper_signature = llvm::FunctionType::get(void_t, false);

    cmain_signature = llvm::FunctionType::get(int_t, {int_t, char_array_ptr_t}, false);

    if (!insert(&Maps::Void,       void_t          ) ||
        !insert(&Maps::Boolean,    boolean_t       ) ||
        !insert(&Maps::Int,        int_t           ) ||
        !insert(&Maps::Float,      double_t        ) ||
        !insert(&Maps::String,     char_array_ptr_t) ||
        !insert(&Maps::IO_Void,    void_t          ) ||
        !insert(&Maps::IO_Boolean, boolean_t       ) ||
        !insert(&Maps::IO_Int,     int_t           ) ||
        !insert(&Maps::IO_Float,   double_t        ) ||
        !insert(&Maps::IO_String,  char_array_ptr_t) ||
        !insert(&Maps::MutString,  mutstring_t     )
    ) {
        Log::compiler_error("Inserting primitive types into TypeMap failed", 
            COMPILER_INIT_SOURCE_LOCATION);
        is_good_ = false;
    }
}

bool TypeMap::contains(const Maps::Type& maps_type) const {
    auto signature = maps_type.name_string();
    return type_map_.contains(signature);
}

bool TypeMap::insert(const Maps::Type* maps_type, llvm::Type* llvm_type) {
    auto signature = maps_type->name_string();
    
    if (contains(*maps_type)) {
        Log::compiler_error(
            "Attempting to store duplicate of \"" + maps_type->name_string() + "\" in TypeMap",
            COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    type_map_.insert({signature, llvm_type});
    return true;
}

// TODO: Would be faster to use type id:s since only primitives have to be covered here
std::optional<llvm::Type*> TypeMap::convert_type(const Maps::Type& type) const {
    auto it = type_map_.find(type.name_string());
    if (it == type_map_.end())
        return std::nullopt;

    return it->second;
}

std::optional<llvm::FunctionType*> TypeMap::convert_function_type(const Maps::Type& return_type, 
    std::span<const Maps::Type* const> arg_types) const {
    
    optional<llvm::Type*> llvm_return_type = convert_type(return_type);
    
    if(!llvm_return_type)
        return nullopt;

    if (*llvm_return_type == mutstring_t)
        llvm_return_type = mutstring_ptr_t;
    
    vector<llvm::Type*> llvm_arg_types{};

    for (auto arg_type: arg_types) {
        // ignore void args
        if (*arg_type == Maps::Void)
            continue;

        optional<llvm::Type*> llvm_arg_type = convert_type(*arg_type);
        if (!llvm_arg_type)
            return nullopt;

        llvm_arg_types.push_back(*llvm_arg_type);
    }

    return llvm::FunctionType::get(*llvm_return_type, llvm_arg_types, false);
}

std::optional<llvm::FunctionType*> TypeMap::convert_function_type(const Maps::FunctionType& type) const {
    if (!type.is_function())
        return nullopt;

    return convert_function_type(*type.return_type(), type.param_types());
}

} // namespace IR
