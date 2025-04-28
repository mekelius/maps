#include "full_parse.hh"

#include <memory>

#include "parser/reverse_parse.hh"
#include "parser_layer1.hh"
#include "parser_layer2.hh"
#include "name_resolution.hh"
#include "lexer.hh"
#include "type_inference.hh"

using std::tuple, std::unique_ptr, std::make_optional, std::nullopt;
using Pragma::Pragmas, Maps::ParserLayer1, Maps::ParserLayer2;

// if parse fails at any point, returns nullopt, 
// except if ignore errors is true returns the broken ast
tuple<bool, unique_ptr<Maps::AST>, unique_ptr<Pragmas>>
    parse_source(std::istream& source_is, const ParseOptions& options, std::ostream& debug_print_ostream) {
            
    std::unique_ptr<Pragma::Pragmas> pragmas = std::make_unique<Pragma::Pragmas>();

    // ----- layer1 -----

    Lexer lexer{&source_is};
    std::unique_ptr<Maps::AST> ast = ParserLayer1{&lexer, pragmas.get(), options.in_repl}.run();

    if (options.print_layer1) {
        debug_print_ostream << "----- layer1 -----";
        reverse_parse(*ast, debug_print_ostream);
    }

    if (!ast->is_valid && options.stop_on_error)
        return {false, std::move(ast), std::move(pragmas)};

    if (options.stop_after == ParseStage::layer1)
        return {true, std::move(ast), std::move(pragmas)};

    // ----- name resolution -----

    if (!resolve_identifiers(*ast) && options.stop_on_error)
        return {false, std::move(ast), std::move(pragmas)};

    // ----- layer2 -----

    ParserLayer2{ast.get(), pragmas.get()}.run();

    if (options.print_layer2) {
        debug_print_ostream << "----- layer2 -----";
        reverse_parse(*ast, debug_print_ostream);
    }

    if (!ast->is_valid && options.stop_on_error)
        return {false, std::move(ast), std::move(pragmas)};

    if (options.stop_after == ParseStage::layer2)
        return {true, std::move(ast), std::move(pragmas)};

    // ----- type checks -----

    if (!Maps::SimpleTypeChecker{}.run(*ast) && options.stop_on_error)
        return {false, std::move(ast), std::move(pragmas)};

    // ----- done -----

    return {true, std::move(ast), std::move(pragmas)};
}
