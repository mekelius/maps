#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Support/TargetSelect.h"

#include "repl.hh"

using std::unique_ptr, std::make_unique;
using Logging::LogLevel;

constexpr std::string_view USAGE = "USAGE: testci [source_file...]";

int main(int argc, char* argv[]) {
    std::vector<std::string> args = {argv + 1, argc + argv};
    std::vector<std::string> source_filenames{};

    REPL::Options repl_options;
    repl_options.eval = true;
    repl_options.print_ir = true;
    repl_options.print_reverse_parse = true;

    for (std::string arg: args) {
        if (arg == "-t" || arg == "--tokens") {
            // lexer_ostream = &std::cout;

        } else if (arg == "--parser-debug") {
            Logging::Settings::set_loglevel(LogLevel::debug);
            
        } else if (arg == "-q" || arg == "--quiet") {
            Logging::Settings::set_loglevel(LogLevel::quiet);

        } else if (arg == "-e" || arg == "--everything") {
            Logging::Settings::set_loglevel(LogLevel::everything);

        } else {
            source_filenames.push_back(arg);
        }
    }

    Logging::init_logging(&std::cerr);
    llvm::raw_os_ostream error_stream{std::cerr};

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
        
    auto ts_context = make_unique<llvm::orc::ThreadSafeContext>(make_unique<llvm::LLVMContext>());
    JIT_Manager jit{ts_context.get(), &error_stream};

    if (!jit.is_good) {
        return EXIT_FAILURE;
    }

    REPL{&jit, ts_context->getContext(), &error_stream, repl_options}.run();
    
    return EXIT_SUCCESS;
}