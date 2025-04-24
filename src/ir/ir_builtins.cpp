#include "ir_builtins.hh"

#include <tuple>

#include "../logging.hh"

namespace IR {

// TODO: parse header file
// TODO: memoize this somehow
bool insert_builtins(IR::IR_Generator& generator) {
    const std::array<std::pair<Maps::Type, llvm::Type*>, 4> PRINTABLE_TYPES{
        std::pair{Maps::String, generator.types_.char_array_ptr_t}, 
        {Maps::Int, generator.types_.int_t}, 
        {Maps::Float, generator.types_.double_t}, 
        {Maps::Boolean, generator.types_.boolean_t}
    };

    // create print types
    for (auto [maps_type, llvm_type]: PRINTABLE_TYPES) {
        if (!generator.function_declaration("print", *generator.maps_types_->get_function_type(Maps::Void, {&maps_type}),
            llvm::FunctionType::get(generator.types_.void_t, {llvm_type}, false))) {
        
            Logging::log_error("Creating builtin functions failed");
            return false;
        }
    }

    return true;
}

} // namespace IR