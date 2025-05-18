#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

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
using Maps::LogInContext, Maps::LogOptions, Maps::LogLevel, Maps::LogContext;
using Maps::CompilationState;

int main(int argc, char* argv[]) {
    llvm::raw_os_ostream error_stream{std::cerr};

    auto log_options_lock = LogOptions::Lock::global();

    REPL::Options repl_options;

    auto [action, exit_code] = process_cl_options(argc, argv, repl_options, 
        *log_options_lock.options_);

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
    if (!REPL{&jit, ts_context->getContext(), &error_stream, repl_options}.run()) 
        return EXIT_FAILURE;
    
    return EXIT_SUCCESS;
}