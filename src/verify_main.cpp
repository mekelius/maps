#include <iostream>
#include <memory>

#include "parser.hh"
#include "lexer.hh"
#include "ast.hh"

int main(int argc, char* argv[]) {


    StreamingLexer lexer{&std::cin, &std::cerr};
    Parser parser{&lexer, &std::cerr};

    std::unique_ptr<AST::AST> ast = parser.run();
    
    return EXIT_SUCCESS;
}