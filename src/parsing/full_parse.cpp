#include "full_parse.hh"

#include <memory>

#include "parser_layer1.hh"
#include "parser_layer2.hh"
#include "name_resolution.hh"
#include "lexer.hh"
#include "type_inference.hh"

using std::tuple, std::unique_ptr, std::make_optional, std::nullopt;
using Pragma::Pragmas, Maps::ParserLayer1, Maps::ParserLayer2;

std::optional<tuple<unique_ptr<Maps::AST>, unique_ptr<Pragmas>>> parse_source(std::istream& source_is, bool in_repl) {    
    std::unique_ptr<Pragma::Pragmas> pragmas = std::make_unique<Pragma::Pragmas>();
    
    Lexer lexer{&source_is};

    std::unique_ptr<Maps::AST> ast = ParserLayer1{&lexer, pragmas.get(), in_repl}.run();
    resolve_identifiers(*ast);
    ParserLayer2{ast.get(), pragmas.get()}.run();

    if (!Maps::SimpleTypeChecker{}.run(*ast))
        return nullopt;

    return make_optional(tuple{std::move(ast), std::move(pragmas)});
}
