#include "process_source.hh"

#include <memory>
#include <optional>
#include <variant>

#include "mapsc/procedures/name_resolution.hh"
#include "mapsc/procedures/reverse_parse.hh"
#include "mapsc/procedures/type_check.hh"

#include "mapsc/parser/parser_layer1.hh"
#include "mapsc/parser/parser_layer2.hh"

using std::tuple, std::optional, std::make_optional, std::nullopt;
using std::unique_ptr, std::make_unique;


namespace Maps {

// if parse fails at any point, returns nullopt, 
// except if ignore errors is true returns the broken ast
std::unique_ptr<CompilationState> process_source(const Scope* builtins, TypeStore* types, 
    std::istream& source_is, const ProcessSourceOptions& options, std::ostream& debug_ostream) {

    auto base = CompilationState{builtins, types};
    return process_source(base, source_is, options, debug_ostream);
}

std::unique_ptr<CompilationState> process_source(const CompilationState& base, std::istream& source_is, 
    const ProcessSourceOptions& options, std::ostream& debug_ostream) {

    unique_ptr<CompilationState> compilation_state = 
        make_unique<CompilationState>(base);
    ReverseParser reverse_parser{
        &debug_ostream, compilation_state->compiler_options_.reverse_parse};
    // ----- layer1 -----

    ParserLayer1 layer1{compilation_state.get()};
    
    if (!options.in_repl) {
        layer1.run(source_is);
    } else {
        layer1.eval_parse(source_is);
    }

    if (options.print_layer1) {
        debug_ostream <<   "\n------- layer1 -------\n\n";
        reverse_parser << *compilation_state;
        debug_ostream << "\n----- layer1 end -----\n\n";
    }

    if (!compilation_state->is_valid && !options.ignore_errors)
        return compilation_state;

    if (options.stop_after == CompilationLayer::layer1)
        return compilation_state;

    // ----- name resolution -----

    if (!resolve_identifiers(*compilation_state, {&compilation_state->globals_}, 
            compilation_state->unresolved_identifiers_) && !options.ignore_errors)
        return compilation_state;

    // ----- layer2 -----

    ParserLayer2{compilation_state.get()}.run();

    if (options.print_layer2) {
        debug_ostream <<   "------- layer2 -------\n\n";
        reverse_parser << *compilation_state;
        debug_ostream << "\n----- layer2 end -----\n\n";
    }

    if (!compilation_state->is_valid && !options.ignore_errors)
        return compilation_state;

    if (options.stop_after == CompilationLayer::layer2)
        return compilation_state;

    // ----- type checks -----

    if (!SimpleTypeChecker{}.run(*compilation_state) && !options.ignore_errors)
        return compilation_state;

    if (options.print_layer3) {
        debug_ostream <<   "------- layer3 -------\n\n";
        reverse_parser << *compilation_state;
        debug_ostream << "\n----- layer3 end -----\n\n";
    }

    return compilation_state;
}

} // namespace Maps