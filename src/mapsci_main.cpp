#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>
#include <filesystem>

#include "pwd.h"
#include "sys/types.h"
#include "unistd.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Support/TargetSelect.h"

// -----------------------------------

#include "logging.hh"

#include "lang/pragma.hh"
#include "repl.hh"

using std::unique_ptr, std::make_unique;
using Logging::LogLevel;

const std::string DATA_SUBDIRECTORY = "mapsc";
const std::string HISTORY_FILENAME = "mapsci_history";

constexpr std::string_view USAGE = "\
!!! NOT UP TO DATE !!!!\n\
\n\
USAGE: mapsci [options]\n\
\n\
options:\n\
  --parser-debug | -q | --quiet | -e | --everything\n\
  --ir | --print-ir\n\
  --no-eval\n\
  --parsed | --print-parsed\n\
  -h | --help\n\
";

// first see if $XDG_DATA_DIR is set
// if not, try $HOME/.local/share
std::optional<std::filesystem::path> get_data_directory() {
    char* c_str;

    if ((c_str = getenv("XDG_DATA_DIR")))
        return std::filesystem::path{c_str};

    std::filesystem::path home_dir;
    if ((c_str = getenv("HOME"))) {
        home_dir = std::filesystem::path{c_str};
    } else {
        std::cerr << "ERROR: finding user data directory failed\n";
        return std::nullopt;
    }

    return home_dir /= ".local/share";
}

// gets the default history file path, creating the subdirectory if it doesn't exist
std::optional<std::filesystem::path> get_default_history_file_path() {
    auto data_directory = get_data_directory();

    if (!data_directory)
        return std::nullopt;
    
    *data_directory /= DATA_SUBDIRECTORY;

    // if the directory doesn't exist, just create it
    if (!std::filesystem::exists(*data_directory))
        std::filesystem::create_directory(*data_directory);

    // check if the directory existed but was a file
    if (!std::filesystem::is_directory(*data_directory)) {
        std::cerr << "ERROR: " << *data_directory << " is not a directory";
        return std::nullopt;
    }

    return *data_directory /= HISTORY_FILENAME;
}

int main(int argc, char* argv[]) {
    Logging::init_logging(&std::cout);
    llvm::raw_os_ostream error_stream{std::cerr};

    std::vector<std::string> args = {argv + 1, argc + argv};
    std::vector<std::string> source_filenames{};

    REPL::Options repl_options;
    repl_options.save_history = true;

    for (std::string arg: args) {
        // split the arg on '='
        std::stringstream arg_s{arg};
        std::string key;
        std::string value;

        std::getline(arg_s, key, '=');
        std::getline(arg_s, value);

        if (key == "-t" || key == "--tokens") {
            Logging::Settings::set_message_type(Logging::MessageType::lexer_debug_token, true);

        } else if (key == "--debug" || key == "--parser-debug") {
            Logging::Settings::set_loglevel(LogLevel::debug);
            
        } else if (key == "-q" || key == "--quiet") {
            Logging::Settings::set_loglevel(LogLevel::quiet);

        } else if (key == "-e" || key == "--everything") {
            Logging::Settings::set_loglevel(LogLevel::everything);

        } else if (key == "--ir" || key == "--print-ir" || key == "--dump-ir") {
            repl_options.print_ir = true;

        } else if(key == "--no-eval") {
            repl_options.eval = false;

        } else if(key == "--parsed" || key == "--print-parsed" || key == "--reverse-parse") {
            repl_options.print_reverse_parse = true;

        } else if(key == "--layer1") {
            repl_options.print_layer1 = true;

        } else if(key == "--layer2") {
            repl_options.print_layer2 = true;

        } else if(key == "--stop_after") {
            if (value == "layer1") {
                repl_options.stop_after = REPL::Stage::layer1;

            } else if (value == "layer2") {
                repl_options.stop_after = REPL::Stage::layer2;

            } else if(key == "ir") {
                repl_options.stop_after = REPL::Stage::ir;

            }
        } else if(key == "--no-history") {
            repl_options.save_history = false;

        } else if(key == "--history-file") {
            repl_options.history_file_path = value;

        } else if(key == "-h" || key == "--help" || key == "--usage") {
            std::cout << USAGE << std::endl;
            return EXIT_SUCCESS;

        } else if (key.at(0) == '-') {
            std::cout << "ERROR: unknown option: " << key << std::endl;
            return EXIT_FAILURE;

        } else {
            std::cerr << "ERROR: interpreting source files not implemented: use mapsc to compile to binary instead, exiting" << std::endl;
            return EXIT_FAILURE;
            source_filenames.push_back(key);
        }
    }

    if (repl_options.save_history && repl_options.history_file_path.empty()) {
        auto history_file_path = get_default_history_file_path();
        if (!history_file_path) {
            repl_options.save_history = false;
            repl_options.history_file_path = "";
        } else {
            repl_options.history_file_path = *history_file_path;
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