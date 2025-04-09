#include <iostream>
#include <memory>
#include <fstream>

#include "config.hh"
#include "parser.hh"
#include "lexer.hh"
#include "ast.hh"
#include "reverse_parse.hh"

constexpr unsigned int OUTPUT_WIDTH = 80;

std::string separator(char character = '-', const std::string& title = "") {
    if (title == "")
        return std::string(OUTPUT_WIDTH, character) + '\n';

    unsigned int lest_side_width = (OUTPUT_WIDTH - title.size() - 2) /2;
    unsigned int right_side_width = lest_side_width + (title.size() % 2);

    return std::string(lest_side_width, character) + ' ' + title + ' ' + std::string(right_side_width, character) + '\n';
}

int main(int argc, char* argv[]) {
    using Logging::LogLevel;
    Logging::init();

    if (argc < 2) {
        std::cerr << "USAGE: verify_mapsc inputfile" << std::endl;
        return EXIT_FAILURE;
    }

    // ----- PARSE CL ARGS -----
    std::vector<std::string> args = { argv + 1, argv + argc };
    std::vector<std::string> source_filenames{};

    std::ostream* lexer_ostream = nullptr;

    Logging::LogLevel::set(LogLevel::default_);

    for (std::string arg : args) {
        if (arg == "--tokens") {
            lexer_ostream = &std::cout;

        } else if (arg == "--parser-debug") {
            Logging::LogLevel::set(LogLevel::everything);
            
        } else if (arg == "-q") {
            Logging::LogLevel::set(LogLevel::quiet);

        } else if (arg == "-v") {
            Logging::LogLevel::set(LogLevel::everything);
            lexer_ostream = &std::cout;

        } else {
            source_filenames.push_back(arg);
        }
    }

    std::cout << separator('#') << separator('#', "VERIFY MAPSC") << "\n" 
              << "Running with " << argc - 1 << " input files\n\n";


    // ----- PROCESS FILES -----
    bool all_succeeded = true;
    for (std::string filename: source_filenames) {
        std::cout << separator('-', filename) << "\n";
        
        std::ifstream source_file{ filename, std::ifstream::in };
        
        if (!source_file) {
            std::cout << "could not open input file: " << filename << std::endl;
            all_succeeded = false;
            continue;
        }
        
        StreamingLexer lexer{&source_file, lexer_ostream};
        Parser parser{&lexer, &std::cerr};
        
        std::unique_ptr<AST::AST> ast = parser.run();
        
        std::cout << "Parsed into this:\n" << std::endl; 
        reverse_parse(*ast, std::cout);
        std::cout << std::endl;
    }

    std::cout << separator();
    
    return all_succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
}