#include <iostream>
#include <memory>
#include <fstream>

#include "mapsc/logging.hh"

#include "mapsc/ast/ast_store.hh"

#include "mapsc/procedures/reverse_parse.hh"
#include "mapsc/parser/parser_layer1.hh"
#include "mapsc/parser/parser_layer2.hh"

#include "mapsc/procedures/name_resolution.hh"

using Maps::LogLevel, Maps::Logger;

constexpr unsigned int OUTPUT_WIDTH = 80;
constexpr std::string_view USAGE = "USAGE: verify_mapsc inputfile... [ -v | --verbose | --parser-debug | --debug | -q | --quiet | -e | --everything ] [ -t | --tokens ]";

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

    Logger::Options logger_options;

    for (std::string arg : args) {
        if (arg == "-t" || arg == "--tokens") {
            // lexer_ostream = &std::cout;

        } else if (arg == "-v" || arg == "--verbose" || arg == "--debug" || arg == "--parser-debug") {
            logger_options.set(LogLevel::debug());
            
        } else if (arg == "-q" || arg == "--quiet") {
            logger_options.set(LogLevel::quiet());

        } else if (arg == "-e" || arg == "--everything") {
            logger_options.set(LogLevel::everything());

        } else {
            source_filenames.push_back(arg);
        }
    }

    Logger::set_global_options(logger_options);

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
        
        std::unique_ptr<Maps::PragmaStore> pragmas = std::make_unique<Maps::PragmaStore>();
        
        std::cout << "run layer1\n\n";
        std::unique_ptr<Maps::AST_Store> ast = std::make_unique<Maps::AST_Store>();

        if (!ast->init_builtins()) {
            Maps::GlobalLogger::log_error("Initializing builtins failed");
            assert(false && "Initializing builtins failed");
            return EXIT_FAILURE;
        }    

        Maps::ParserLayer1{ast.get(), pragmas.get()}.run(source_file);

        if (Logger::logs_since_last_check()) 
            std::cout << "\n";
        if (!ast->is_valid) {
            std::cerr << "layer1 failed\n\n";
            continue;
        }
        std::cout << "layer1 done\n\n";
        std::cout << "run name resolution\n\n";

        resolve_identifiers(*ast);

        if(Logger::logs_since_last_check()) 
            std::cout << "\n";
        if (!ast->is_valid) {
            std::cerr << "name resolution failed\n\n";
            continue;
        }
        std::cout << "name resolution done\n\n";
        std::cout << "run layer 2\n\n";

        Maps::ParserLayer2{ast.get(), pragmas.get()}.run();

        if(Logger::logs_since_last_check()) 
            std::cout << "\n";
        if (!ast->is_valid) {
            std::cerr << "layer2 failed\n\n";
            continue;
        }
        std::cout << "layer2 done\n\n";

        std::cout << separator('-', "Parsed into"); 
        Maps::ReverseParser{&std::cout} << *ast;
        std::cout << "\n" << std::endl;
    }

    std::cout << separator();
    
    return all_succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
}