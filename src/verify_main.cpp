#include <iostream>
#include <memory>
#include <fstream>

#include "logging.hh"

#include "lang/ast.hh"
#include "lang/reverse_parse.hh"

#include "parsing/parser_layer1.hh"
#include "parsing/parser_layer2.hh"

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
    using Logging::LogLevel;
    Logging::init_logging(&std::cout);

    if (argc < 2) {
        std::cerr << USAGE << std::endl;
        return EXIT_FAILURE;
    }

    // ----- PARSE CL ARGS -----
    std::vector<std::string> args = { argv + 1, argv + argc };
    std::vector<std::string> source_filenames{};

    std::ostream* lexer_ostream = nullptr;

    Logging::Settings::set_loglevel(LogLevel::default_);

    for (std::string arg : args) {
        if (arg == "-t" || arg == "--tokens") {
            lexer_ostream = &std::cout;

        } else if (arg == "-v" || arg == "--verbose" || arg == "--debug" || arg == "--parser-debug") {
            Logging::Settings::set_loglevel(LogLevel::debug);
            
        } else if (arg == "-q" || arg == "--quiet") {
            Logging::Settings::set_loglevel(LogLevel::quiet);

        } else if (arg == "-e | --everything") {
            Logging::Settings::set_loglevel(LogLevel::everything);
            lexer_ostream = &std::cout;

        } else {
            source_filenames.push_back(arg);
        }
    }

    std::cout << separator('#') << separator('#', "VERIFY MAPSC") << "\n" 
              << "Running with " << source_filenames.size() << " input files\n\n";


    // ----- PROCESS FILES -----
    bool all_succeeded = true;
    for (std::string filename: source_filenames) {
        std::cout << separator('#') << separator(' ', filename) << "\n";
        
        std::ifstream source_file{ filename, std::ifstream::in };
        
        if (!source_file) {
            std::cout << "could not open input file: " << filename << std::endl;
            all_succeeded = false;
            continue;
        }
        
        StreamingLexer lexer{&source_file};
        std::unique_ptr<Pragma::Pragmas> pragmas = std::make_unique<Pragma::Pragmas>();
        ParserLayer1 layer1{&lexer, pragmas.get()};
        
        std::unique_ptr<AST::AST> ast = layer1.run();
        std::cout << "\nlayer1 done" << std::endl;

        ParserLayer2 layer2{ast.get(), pragmas.get()};
        layer2.run();
        std::cout << "layer2 done\n" << std::endl;

        std::cout << separator('-', "Parsed into") << "\n" << std::endl; 
        reverse_parse(*ast, std::cout);
        std::cout << std::endl;
    }

    std::cout << separator();
    
    return all_succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
}