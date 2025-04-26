#include "full_parse.hh"

#include <memory>

#include "parser_layer1.hh"
#include "parser_layer2.hh"
#include "name_resolution.hh"
#include "lexer.hh"
#include "type_inference.hh"

using std::tuple, std::unique_ptr, std::make_optional, std::nullopt;
using Pragma::Pragmas, Maps::ParserLayer1, Maps::ParserLayer2;

// if parse fails at any point, returns nullopt, 
// except if ignore errors is true returns the broken ast
std::optional<tuple<unique_ptr<Maps::AST>, unique_ptr<Pragmas>>> parse_source(std::istream& source_is, 
    bool in_repl, bool ignore_errors) {
    
    std::unique_ptr<Pragma::Pragmas> pragmas = std::make_unique<Pragma::Pragmas>();
    
    Lexer lexer{&source_is};
    std::unique_ptr<Maps::AST> ast = ParserLayer1{&lexer, pragmas.get(), in_repl}.run();

    if (!ast->is_valid)
        return !ignore_errors ? nullopt : make_optional(tuple{std::move(ast), std::move(pragmas)});

    if (!resolve_identifiers(*ast))
        return !ignore_errors ? nullopt : make_optional(tuple{std::move(ast), std::move(pragmas)});

    ParserLayer2{ast.get(), pragmas.get()}.run();
    if (!ast->is_valid && !ignore_errors)
        return !ignore_errors ? nullopt : make_optional(tuple{std::move(ast), std::move(pragmas)});

    if (!Maps::SimpleTypeChecker{}.run(*ast) && !ignore_errors)
        return !ignore_errors ? nullopt : make_optional(tuple{std::move(ast), std::move(pragmas)});

    return make_optional(tuple{std::move(ast), std::move(pragmas)});
}
