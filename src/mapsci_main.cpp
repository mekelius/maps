#include <cstdlib>
#include <iostream>
#include <memory>

#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_os_ostream.h"

#include "mapsc/logging.hh"
#include "mapsc/logging_options.hh"

#include "mapsci/cl_options.hh"
#include "mapsci/init_llvm.hh"
#include "mapsci/jit_manager.hh"
#include "mapsci/repl.hh"


using std::unique_ptr, std::make_unique;
using Maps::LogInContext, Maps::LogOptions, Maps::LogLevel, Maps::LogContext, Maps::LogStream;
using Maps::CompilationState;
using Maps::JIT_Manager, Maps::SHOULD_EXIT;

int main(int argc, char* argv[]) {
    llvm::raw_os_ostream error_stream{std::cerr};

    auto log_options_lock = LogStream::global.lock();
    auto [action, exit_code, repl_options] = process_cl_options(argc, argv, *log_options_lock.options_);

    if (action == SHOULD_EXIT)
        return exit_code;

    if (!init_llvm()) {
        std::cerr << "initializing llvm failed" << std::endl;
        return EXIT_FAILURE;
    }

    auto ts_context = make_unique<llvm::orc::ThreadSafeContext>(make_unique<llvm::LLVMContext>());
    JIT_Manager jit{ts_context.get(), &error_stream};

    if (!jit.is_good) {
        std::cerr << "initializing JIT failed" << std::endl;
        return EXIT_FAILURE;
    }

    // REPL logs its own failures
    if (!run_repl(jit, *ts_context->getContext(), error_stream, repl_options)) 
        return EXIT_FAILURE;
    
    return EXIT_SUCCESS;
}