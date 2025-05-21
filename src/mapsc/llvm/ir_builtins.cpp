#include "ir_builtins.hh"

#include <array>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

#include "mapsc/logging.hh"

#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/types/type_store.hh"

#include "mapsc/llvm/ir_generator.hh"
#include "mapsc/llvm/type_mapping.hh"

using std::optional;
using Log = Maps::LogInContext<Maps::LogContext::ir_gen_init>;

namespace llvm { class Type; }
namespace Maps { class FunctionType; }

namespace IR {

// TODO: parse header file
// TODO: memoize this somehow
bool insert_builtins(IR::IR_Generator& generator) {
    const std::array<std::pair<const Maps::Type*, llvm::Type*>, 4> PRINTABLE_TYPES{
       std::pair{&Maps::String, generator.types_.char_array_ptr_t}, 
                {&Maps::Int, generator.types_.int_t}, 
                {&Maps::Float, generator.types_.double_t}, 
                {&Maps::Boolean, generator.types_.boolean_t}
    };

    // create print types
    for (auto [maps_type, llvm_type]: PRINTABLE_TYPES) {
        if (!generator.forward_declaration("print", *generator.maps_types_->get_function_type(
                Maps::IO_Void, {maps_type}, false),
                llvm::FunctionType::get(generator.types_.void_t, {llvm_type}, false))) {

            Log::compiler_error("Creating builtin print functions failed", 
                COMPILER_INIT_SOURCE_LOCATION);
            return false;
        }
    }

    // arithmetic function types
    const Maps::FunctionType* IntInt = generator.maps_types_->get_function_type(
        Maps::Int, {&Maps::Int}, true);
    llvm::FunctionType* llvm_IntInt = llvm::FunctionType::get(generator.types_.int_t, 
        {generator.types_.int_t}, false);

    const Maps::FunctionType* IntIntInt = generator.maps_types_->get_function_type(
        Maps::Int, {&Maps::Int, &Maps::Int}, true);
    llvm::FunctionType* llvm_IntIntInt = llvm::FunctionType::get(generator.types_.int_t, 
        {generator.types_.int_t, generator.types_.int_t}, false);
    
    const Maps::FunctionType* maps_FloatFloatFloat = generator.maps_types_->get_function_type(
        Maps::Float, {&Maps::Float, &Maps::Float}, true);
    llvm::FunctionType* llvm_FloatFloatFloat = llvm::FunctionType::get(generator.types_.double_t, 
        {generator.types_.double_t, generator.types_.double_t}, false);

    llvm::Value* zero = llvm::ConstantInt::get(*generator.context_, llvm::APInt(32, 0, true));
    
    optional<llvm::Function*> cast_Boolean_to_String = generator.function_definition("to_String", 
        *generator.maps_types_->get_function_type(Maps::String, {&Maps::Boolean}, true), 
        llvm::FunctionType::get(generator.types_.char_array_ptr_t, {generator.types_.boolean_t}, 
            false));
    
    if (!cast_Boolean_to_String) {
        Log::compiler_error("Creating builtin cast from Boolean to const String failed", 
            COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    generator.builder_->CreateRet(generator.builder_->CreateSelect(
        generator.builder_->getInt1((*cast_Boolean_to_String)->getArg(0)),
            generator.builder_->CreateGlobalString("true"),
            generator.builder_->CreateGlobalString("false")
    ));

    // ##########  -Int  ##########
    optional<llvm::Function*> negate_int = generator.function_definition("-", *IntInt, llvm_IntInt);

    if (!negate_int) {
        Log::compiler_error("creating builtin unary - failed", COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateSub(zero, (*negate_int)->getArg(0))
    );

    // ##########  Int + Int  ##########
    optional<llvm::Function*> int_add = generator.function_definition("+", *IntIntInt, llvm_IntIntInt);

    if (!int_add) {
        Log::compiler_error("creating builtin + failed", COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateAdd(
            (*int_add)->getArg(0), (*int_add)->getArg(1)
        )
    );

    // ##########  Int * Int  ##########
    optional<llvm::Function*> int_mul = generator.function_definition("*", *IntIntInt, llvm_IntIntInt);

    if (!int_mul) {
        Log::error("creating builtin * failed", COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateMul(
            (*int_mul)->getArg(0), (*int_mul)->getArg(1)
        )
    );

    // ##########  Int - Int  ##########
    optional<llvm::Function*> int_sub = generator.function_definition("-", *IntIntInt, llvm_IntIntInt);

    if (!int_sub) {
        Log::error("creating builtin - failed", COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateSub(
            (*int_sub)->getArg(0), (*int_sub)->getArg(1)
        )
    );

    // ##########  Float + Float  ##########
    optional<llvm::Function*> float_add = generator.function_definition(
        "+", *maps_FloatFloatFloat, llvm_FloatFloatFloat);

    if (!float_add) {
        Log::error("creating builtin + failed", COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateFAdd(
            (*float_add)->getArg(0), (*float_add)->getArg(1)
        )
    );

    // ##########  Float * Float  ##########
    optional<llvm::Function*> float_mul = generator.function_definition(
        "*", *maps_FloatFloatFloat, llvm_FloatFloatFloat);

    if (!float_mul) {
        Log::error("creating builtin * failed", COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateFMul(
            (*float_mul)->getArg(0), (*float_mul)->getArg(1)
        )
    );

    // ##########  Float - Float  ##########
    optional<llvm::Function*> float_sub = generator.function_definition(
        "-", *maps_FloatFloatFloat, llvm_FloatFloatFloat);

    if (!float_sub) {
        Log::error("creating builtin - failed", COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateFSub(
            (*float_sub)->getArg(0), (*float_sub)->getArg(1)
        )
    );

    // ##########  Float / Float  ##########
    optional<llvm::Function*> float_div = generator.function_definition(
        "/", *maps_FloatFloatFloat, llvm_FloatFloatFloat);

    if (!float_div) {
        Log::error("creating builtin - failed", COMPILER_INIT_SOURCE_LOCATION);
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateFDiv(
            (*float_div)->getArg(0), (*float_div)->getArg(1)
        )
    );

    return true;
}

} // namespace IR