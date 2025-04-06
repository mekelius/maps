#include <iostream>
#include <memory>

#include "parser.hh"
#include "lexer.hh"

int main(int argc, char* argv[]) {
    std::unique_ptr<StreamingLexer> lexer = std::make_unique<StreamingLexer>(&std::cin, &std::cerr);
    
    return EXIT_SUCCESS;
}