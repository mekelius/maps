#ifndef __IR_BUILTINS_HH
#define __IR_BUILTINS_HH

namespace llvm {

class Function;

} // namespace llvm

namespace Maps {
namespace LLVM_IR {

class IR_Generator;

struct Builtins {
    // builtin functions
    ::llvm::Function* print;
};

// declares builtins and external functions used by them
bool insert_builtins(IR_Generator& generator);

} // namespace LLVM_IR
} // nameespace Maps

#endif