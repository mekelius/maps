#ifndef __IR_BUILTINS_HH
#define __IR_BUILTINS_HH

#include "llvm/IR/Function.h"

#include "mapsc/llvm/ir_generator.hh"

namespace IR {

struct Builtins {
    // builtin functions
    llvm::Function* print;
};

// declares builtins and external functions used by them
bool insert_builtins(IR_Generator& generator);

} // namespace IR

#endif