#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <fstream>

#include "mapsc/logging.hh"
#include "mapsc/logging_options.hh"

#include "mapsc/builtins.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/types/type_store.hh"

#include "mapsc/parser/parser_layer1.hh"
#include "mapsc/parser/parser_layer2.hh"

#include "mapsc/procedures/reverse_parse.hh"
#include "mapsc/procedures/name_resolution.hh"


using Maps::LogInContext, Maps::LogOptions, Maps::LogLevel, Maps::LogContext;

constexpr unsigned int OUTPUT_WIDTH = 80;
constexpr std::string_view USAGE = "\n\
USAGE: verify_mapsc inputfile... [options]\n\
\n\
options:\n\
    -v | --verbose | --parser-debug | --debug | -q | --quiet | -e | --everything\n\
    -t | --tokens\n\
";

std::string separator(char character = '-', const std::string& title = "") {
    if (title == "")
        return std::string(OUTPUT_WIDTH, character) + '\n';

    unsigned int lest_side_width = (OUTPUT_WIDTH - title.size() - 2) /2;
    unsigned int right_side_width = lest_side_width + (title.size() % 2);

    return std::string(lest_side_width, character) + ' ' + title + ' ' + std::string(right_side_width, character) + '\n';
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << USAGE << std::endl;
        return EXIT_FAILURE;
    }

    // ----- PARSE CL ARGS -----
    std::vector<std::string> args = { argv + 1, argv + argc };
    std::vector<std::string> source_filenames{};

    auto log_options_lock = LogOptions::Lock::global();
    auto log_options = log_options_lock.options_;

    for (std::string arg : args) {
        if (arg == "-t" || arg == "--tokens") {
            // lexer_ostream = &std::cout;

        } else if (arg == "-v" || arg == "--verbose" || arg == "--debug" || arg == "--parser-debug") {
            log_options->set_loglevel(LogLevel::debug);
            
        } else if (arg == "-q" || arg == "--quiet") {
            log_options->set_loglevel(LogLevel::compiler_error);

        } else if (arg == "-e" || arg == "--everything") {
            log_options->set_loglevel(LogLevel::debug_extra);

        } else {
            source_filenames.push_back(arg);
        }
    }

    std::cout << separator('#') << separator('#', "VERIFY MAPSC") << "\n" 
              << "Running with " << source_filenames.size() << " input files\n\n";


    // ----- PROCESS FILES -----
    bool all_succeeded = true;
    for (std::string filename: source_filenames) {
        std::cout << separator('#') << "\n" << separator(' ', filename) << "\n\n";
        
        std::ifstream source_file{ filename, std::ifstream::in };
        
        if (!source_file) {
            std::cout << "could not open input file: " << filename << std::endl;
            all_succeeded = false;
            continue;
        }
        
        std::cout << "run layer1\n\n";

        Maps::TypeStore types{};
        Maps::CompilationState compilation_state{Maps::get_builtins(), &types};

        Maps::ParserLayer1{&compilation_state}.run(source_file);

        if (Maps::logs_since_last_check()) 
            std::cout << "\n";
        if (!compilation_state.is_valid) {
            std::cerr << "layer1 failed\n\n";
            continue;
        }
        std::cout << "layer1 done\n\n";
        std::cout << "run name resolution\n\n";

        resolve_identifiers(compilation_state, {&compilation_state.globals_}, 
            compilation_state.unresolved_identifiers_);

        if(Maps::logs_since_last_check()) 
            std::cout << "\n";
        if (!compilation_state.is_valid) {
            std::cerr << "name resolution failed\n\n";
            continue;
        }
        std::cout << "name resolution done\n\n";
        std::cout << "run layer 2\n\n";

        Maps::ParserLayer2{&compilation_state}.run();

        if(Maps::logs_since_last_check()) 
            std::cout << "\n";
        if (!compilation_state.is_valid) {
            std::cerr << "layer2 failed\n\n";
            continue;
        }
        std::cout << "layer2 done\n\n";

        std::cout << separator('-', "Parsed into"); 
        Maps::ReverseParser{&std::cout} << compilation_state;
        std::cout << "\n" << std::endl;
    }

    std::cout << separator();
    
    return all_succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
}