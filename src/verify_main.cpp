#include <iostream>
#include <memory>
#include <fstream>

#include "parser.hh"
#include "lexer.hh"
#include "ast.hh"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "USAGE: verify_mapsc inputfile" << std::endl;
        return EXIT_FAILURE;
    }

    std::string source_filename = argv[1];

    std::ifstream source_file{ source_filename, std::ifstream::in };

    if (!source_file) {
        std::cerr << "FATAL: could not open input file: " << source_filename << std::endl;
        return EXIT_FAILURE;
    }

    StreamingLexer lexer{&source_file, &std::cerr};
    Parser parser{&lexer, &std::cerr};

    std::unique_ptr<AST::AST> ast = parser.run();
    
    return EXIT_SUCCESS;
}