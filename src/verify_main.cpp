#include <iostream>
#include <memory>
#include <fstream>

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
    if (argc < 2) {
        std::cerr << "USAGE: verify_mapsc inputfile" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << separator('#') << separator('#', "VERIFY MAPSC") << "\n" 
              << "Running with " << argc - 1 << " input files\n\n";

    std::vector<std::string> source_filenames = { argv + 1, argv + argc };

    bool all_succeeded = true;
    
    for (std::string filename: source_filenames) {
        std::cout << separator('-', filename) << "\n";
        
        std::ifstream source_file{ filename, std::ifstream::in };
        
        if (!source_file) {
            std::cout << "could not open input file: " << filename << std::endl;
            all_succeeded = false;
            continue;
        }
        
        StreamingLexer lexer{&source_file};
        Parser parser{&lexer, &std::cerr};
        
        std::unique_ptr<AST::AST> ast = parser.run();
        
        std::cout << "Parsed into this:\n" << std::endl; 
        reverse_parse(*ast, std::cout);
        std::cout << std::endl;
    }

    std::cout << separator();
    
    return all_succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
}