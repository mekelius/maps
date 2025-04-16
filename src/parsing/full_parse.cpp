#include "full_parse.hh"

#include <memory>

#include "parser_layer1.hh"
#include "parser_layer2.hh"
#include "name_resolution.hh"
#include "lexer.hh"

using std::tuple, std::unique_ptr, Pragma::Pragmas;

tuple<unique_ptr<AST::AST>, unique_ptr<Pragmas>> parse_source(std::istream& source_is) {    
    std::unique_ptr<Pragma::Pragmas> pragmas = std::make_unique<Pragma::Pragmas>();
    
    StreamingLexer lexer{&source_is};

    std::unique_ptr<AST::AST> ast = ParserLayer1{&lexer, pragmas.get()}.run();
    resolve_identifiers(*ast);
    ParserLayer2{ast.get(), pragmas.get()}.run();

    return {std::move(ast), std::move(pragmas)};
}
