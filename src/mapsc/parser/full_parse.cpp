#include "full_parse.hh"

#include <memory>

#include "mapsc/parser/lexer.hh"
#include "mapsc/parser/parser_layer1.hh"
#include "mapsc/parser/parser_layer2.hh"
#include "mapsc/procedures/name_resolution.hh"
#include "mapsc/procedures/type_check.hh"
#include "mapsc/procedures/reverse_parse.hh"
#include "mapsc/procedures/concretize.hh"

using std::tuple, std::optional, std::make_optional, std::nullopt;
using std::unique_ptr, std::make_unique;

namespace Maps {

// if parse fails at any point, returns nullopt, 
// except if ignore errors is true returns the broken ast
tuple<bool, unique_ptr<AST_Store>, unique_ptr<PragmaStore>>
    parse_source(std::istream& source_is, const ParseOptions& options, std::ostream& debug_ostream) {

    unique_ptr<PragmaStore> pragmas = make_unique<PragmaStore>();
    unique_ptr<AST_Store> ast = make_unique<AST_Store>();
    ReverseParser reverse_parser{&debug_ostream};
    
    if (!ast->init_builtins()) {
        GlobalLogger::log_error("Initializing builtins failed");
        assert(false && "Initializing builtins failed");
        return {false, std::move(ast), std::move(pragmas)};
    }

    // ----- layer1 -----

    ParserLayer1 layer1{ast.get(), pragmas.get()};
    
    if (!options.in_repl) {
        layer1.run(source_is);
    } else {
        layer1.eval_parse(source_is);
    }

    if (options.print_layer1) {
        debug_ostream <<   "\n------- layer1 -------\n\n";
        reverse_parser << *ast;
        debug_ostream << "\n----- layer1 end -----\n\n";
    }

    if (!ast->is_valid && !options.ignore_errors)
        return {false, std::move(ast), std::move(pragmas)};

    if (options.stop_after == ParseStage::layer1)
        return {true, std::move(ast), std::move(pragmas)};

    // ----- name resolution -----

    if (!resolve_identifiers(*ast) && !options.ignore_errors)
        return {false, std::move(ast), std::move(pragmas)};

    // ----- layer2 -----

    ParserLayer2{ast.get(), pragmas.get()}.run();

    if (options.print_layer2) {
        debug_ostream <<   "------- layer2 -------\n\n";
        reverse_parser << *ast;
        debug_ostream << "\n----- layer2 end -----\n\n";
    }

    if (!ast->is_valid && !options.ignore_errors)
        return {false, std::move(ast), std::move(pragmas)};

    if (options.stop_after == ParseStage::layer2)
        return {true, std::move(ast), std::move(pragmas)};

    // ----- type checks -----

    if (!SimpleTypeChecker{}.run(*ast) && !options.ignore_errors)
        return {false, std::move(ast), std::move(pragmas)};

    // ----- done -----

    return {true, std::move(ast), std::move(pragmas)};
}

} // namespace Maps