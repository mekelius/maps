#include "repl.hh"
#include "repl/implementation.hh"

namespace Maps {

bool run_repl(JIT_Manager& jit, llvm::LLVMContext& context, llvm::raw_ostream& error_stream, 
    const REPL_Options& options) {
    
    return REPL{&jit, &context, &error_stream, options}.run();
}

} // namespace Maps
