#include "ir_builtins.hh"

#include <tuple>
#include <optional>

#include "mapsc/logging.hh"

using std::optional;
using Logging::log_error;

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
        if (!generator.function_declaration("print", *generator.maps_types_->get_function_type(
                Maps::Void, {maps_type}),
            llvm::FunctionType::get(generator.types_.void_t, {llvm_type}, false))) {
        
            Logging::log_error("Creating builtin functions failed");
            return false;
        }
    }

    // insert arithmetic functions
    const Maps::FunctionType* maps_IntIntInt = generator.maps_types_->get_function_type(
        Maps::Int, {&Maps::Int, &Maps::Int});
    llvm::FunctionType* llvm_IntIntInt = llvm::FunctionType::get(generator.types_.int_t, {generator.types_.int_t, generator.types_.int_t}, false);
    
    const Maps::FunctionType* maps_FloatFloatFloat = generator.maps_types_->get_function_type(
        Maps::Float, {&Maps::Float, &Maps::Float});
    llvm::FunctionType* llvm_FloatFloatFloat = llvm::FunctionType::get(generator.types_.double_t, {generator.types_.double_t, generator.types_.double_t}, false);
    
    optional<llvm::Function*> int_add = generator.function_definition("+", *maps_IntIntInt, llvm_IntIntInt);

    if (!int_add) {
        log_error("creating builtin + failed");
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateAdd(
            (*int_add)->getArg(0), (*int_add)->getArg(1)
        )
    );

    optional<llvm::Function*> int_mul = generator.function_definition("*", *maps_IntIntInt, llvm_IntIntInt);

    if (!int_mul) {
        log_error("creating builtin * failed");
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateMul(
            (*int_mul)->getArg(0), (*int_mul)->getArg(1)
        )
    );

    optional<llvm::Function*> int_sub = generator.function_definition("-", *maps_IntIntInt, llvm_IntIntInt);

    if (!int_sub) {
        log_error("creating builtin - failed");
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateSub(
            (*int_sub)->getArg(0), (*int_sub)->getArg(1)
        )
    );

    optional<llvm::Function*> float_add = generator.function_definition("+", *maps_FloatFloatFloat, llvm_FloatFloatFloat);

    if (!float_add) {
        log_error("creating builtin + failed");
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateFAdd(
            (*float_add)->getArg(0), (*float_add)->getArg(1)
        )
    );

    optional<llvm::Function*> float_mul = generator.function_definition("*", *maps_FloatFloatFloat, llvm_FloatFloatFloat);

    if (!float_mul) {
        log_error("creating builtin * failed");
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateFMul(
            (*float_mul)->getArg(0), (*float_mul)->getArg(1)
        )
    );

    optional<llvm::Function*> float_sub = generator.function_definition("-", *maps_FloatFloatFloat, llvm_FloatFloatFloat);

    if (!float_sub) {
        log_error("creating builtin - failed");
        return false;
    }

    generator.builder_->CreateRet(
        generator.builder_->CreateFSub(
            (*float_sub)->getArg(0), (*float_sub)->getArg(1)
        )
    );

    optional<llvm::Function*> float_div = generator.function_definition("/", *maps_FloatFloatFloat, llvm_FloatFloatFloat);

    if (!float_div) {
        log_error("creating builtin - failed");
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