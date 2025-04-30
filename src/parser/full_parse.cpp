#include "full_parse.hh"

#include <memory>

#include "parser/lexer.hh"
#include "parser/reverse_parse.hh"
#include "parser/parser_layer1.hh"
#include "parser/type_declaration.hh"
#include "parser/name_resolution.hh"
#include "parser/parser_layer2.hh"
#include "parser/type_checking.hh"

using std::tuple, std::unique_ptr, std::make_optional, std::nullopt;
using std::unique_ptr, std::make_unique;
using Maps::Pragmas, Maps::ParserLayer1, Maps::ParserLayer2, Maps::SimpleTypeChecker;
using Maps::handle_BTD_field_names, Maps::resolve_identifiers;

// if parse fails at any point, returns nullopt, 
// except if ignore errors is true returns the broken ast
tuple<bool, unique_ptr<Maps::AST>, unique_ptr<Pragmas>>
    parse_source(std::istream& source_is, const ParseOptions& options, std::ostream& debug_print_ostream) {
            
    std::unique_ptr<Pragmas> pragmas = std::make_unique<Pragmas>();
    unique_ptr<Maps::AST> ast = make_unique<Maps::AST>(); 
    
    if (!ast->init_builtins()) {
        Logging::log_error("Initializing builtins failed");
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
        debug_print_ostream <<   "\n------- layer1 -------";
        reverse_parse(*ast, debug_print_ostream);
        debug_print_ostream << "\n----- layer1 end -----\n\n";
    }

    if (!ast->is_valid && options.stop_on_error)
        return {false, std::move(ast), std::move(pragmas)};

    if (options.stop_after == ParseStage::layer1)
        return {true, std::move(ast), std::move(pragmas)};

    // ----- binding type declarations -----

    if (!handle_BTD_field_names(ast->possible_binding_type_declarations_) && options.stop_on_error)
        return {false, std::move(ast), std::move(pragmas)};

    // ----- name resolution -----

    if (!resolve_identifiers(*ast) && options.stop_on_error)
        return {false, std::move(ast), std::move(pragmas)};

    // ----- layer2 -----

    ParserLayer2{ast.get(), pragmas.get()}.run();

    if (options.print_layer2) {
        debug_print_ostream <<   "------- layer2 -------";
        reverse_parse(*ast, debug_print_ostream);
        debug_print_ostream << "\n----- layer2 end -----\n\n";
    }

    if (!ast->is_valid && options.stop_on_error)
        return {false, std::move(ast), std::move(pragmas)};

    if (options.stop_after == ParseStage::layer2)
        return {true, std::move(ast), std::move(pragmas)};

    // ----- type checks -----

    if (!SimpleTypeChecker{}.run(*ast) && options.stop_on_error)
        return {false, std::move(ast), std::move(pragmas)};

    // ----- done -----

    return {true, std::move(ast), std::move(pragmas)};
}
