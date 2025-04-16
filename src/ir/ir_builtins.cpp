#include "ir_builtins.hh"

namespace IR {

void insert_builtins(IR::IR_Generator& generator) {
    // TODO: figure out string types
    // declare needed c stdlib functions
    llvm::Function* puts = generator.function_declaration("puts", 
        llvm::FunctionType::get(generator.types_.int_t, {generator.types_.char_array_ptr_t}, false));

    llvm::Function* sprintf_ = generator.function_declaration("sprintf", 
        llvm::FunctionType::get(generator.types_.void_t, {generator.types_.char_array_13_t, generator.types_.char_array_13_t, generator.types_.double_t}, true));
        
    // TODO: write print in c
    llvm::Function* print = puts;
}

} // namespace IR