#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Support/TargetSelect.h"

#include "logging.hh"

#include "lang/pragma.hh"
#include "repl.hh"

using std::unique_ptr, std::make_unique;
using Logging::LogLevel;

constexpr std::string_view USAGE = "\
\nUSAGE: mapsci [options]\n\n\
options:\n\
  --parser-debug | -q | --quiet | -e | --everything\n  --ir | --print-ir\n\
  --no-eval\n\
  --parsed | --print-parsed\n\
  -h | --help\n\
";

int main(int argc, char* argv[]) {
    Logging::init_logging(&std::cout);
    llvm::raw_os_ostream error_stream{std::cerr};

    std::vector<std::string> args = {argv + 1, argc + argv};
    std::vector<std::string> source_filenames{};

    REPL::Options repl_options;

    for (std::string arg: args) {
        if (arg == "-t" || arg == "--tokens") {
            Logging::Settings::set_message_type(Logging::MessageType::lexer_debug_token, true);

        } else if (arg == "--debug" || arg == "--parser-debug") {
            Logging::Settings::set_loglevel(LogLevel::debug);
            
        } else if (arg == "-q" || arg == "--quiet") {
            Logging::Settings::set_loglevel(LogLevel::quiet);

        } else if (arg == "-e" || arg == "--everything") {
            Logging::Settings::set_loglevel(LogLevel::everything);

        } else if (arg == "--ir" || arg == "--print-ir" || arg == "--dump-ir") {
            repl_options.print_ir = true;

        } else if(arg == "--no-eval") {
            repl_options.eval = false;

        } else if(arg == "--parsed" || arg == "--print-parsed" || arg == "--reverse-parse") {
            repl_options.print_reverse_parse = true;

        } else if(arg == "--layer1") {
            repl_options.print_layer1 = true;

        } else if(arg == "--layer2") {
            repl_options.print_layer2 = true;

        } else if(arg == "--stop_after=layer1") {
            repl_options.stop_after = REPL::Stage::layer1;

        } else if(arg == "--stop_after=layer2") {
            repl_options.stop_after = REPL::Stage::layer2;
        
        } else if(arg == "--stop_after=ir") {
            repl_options.stop_after = REPL::Stage::ir;

        } else if(arg == "-h" || arg == "--help" || arg == "--usage") {
            std::cout << USAGE << std::endl;
            return EXIT_SUCCESS;

        } else if (arg.at(0) == '-') {
            std::cout << "ERROR: unknown option: " << arg << std::endl;
            return EXIT_FAILURE;

        } else {
            std::cerr << "ERROR: interpreting source files not implemented: use mapsc to compile to binary instead, exiting" << std::endl;
            return EXIT_FAILURE;
            source_filenames.push_back(arg);
        }
    }

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