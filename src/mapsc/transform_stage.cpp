#include "transform_stage.hh"

#include <span>

#include "mapsc/logging.hh"

#include "mapsc/ast/scope.hh"
#include "mapsc/ast/definition.hh"

#include "mapsc/procedures/concretize.hh"
#include "mapsc/compilation_state.hh"

namespace Maps {

using Log = LogInContext<LogContext::transform_stage>;

bool run_transforms(CompilationState& state, Scope& scope, 
    std::span<DefinitionBody* const> definitions) {

    for (auto definition: definitions) {
        if (!run_transforms(state, scope, *definition))
            return false;
    }

    return true;
}

// class Transformer {
//     using Transforms = std::tuple<std::vector<
    
//     Transformer()

//     bool visit_definition() {

//     }

//     bool visit_statement() {

//     }

//     bool visit_expression() {

//     }
// }

bool run_transforms(CompilationState& state, Scope& scope, DefinitionBody& definition) {
    if (definition.is_deleted()) {
        Log::debug_extra("Skipping transforms on deleted definition " + definition.name_string(), 
            definition.location());
        return true;
    }

    if (!definition.is_top_level_definition()) {
        Log::compiler_error(
            "run_transforms called on not a top level definition", definition.location());
        assert(false && "run_transforms called on not a top level definition");
        return false;
    }

    Log::debug_extra("Running transforms on definition " + definition.name_string() + "...", 
        definition.location());

    // Log::debug_extra("Simplify " + definition.to_string() + "...", 
    //     definition.location());
    // if (!simplify(definition)) {
    //     Log::error("Simplifying " + definition.to_string() + " failed", definition.location());
    //     return false;
    // }
    // Log::debug_extra("Simplify ok", 
    //     definition.location());

    
    // Log::debug_extra("Inlines & substitutions on " + definition.to_string() + "...", 
    //     definition.location());
    // if (!inline_and_substitute(definition)) {
    //     Log::error("Inlines & substitutions on " + definition.to_string() + " failed", definition.location());
    //     return false;
    // }
    // Log::debug_extra("Inlines & substitutions ok", 
    //     definition.location());


    Log::debug_extra("Concretize " + definition.name_string() + "...", 
        definition.location());
    if (!concretize(state, definition)) {
        Log::error("Concretizing " + definition.name_string() + " failed", definition.location());
        return false;
    }
    Log::debug_extra("Concretize ok", 
        definition.location());

    
    // Log::debug_extra("Type checking " + definition.to_string() + "...", 
    //     definition.location());
    // if (!type_check(definition)) {
    //     Log::error("Type check on " + definition.to_string() + " failed", definition.location());
    //     return false;
    // }
    // Log::debug_extra("Type check ok", 
    //     definition.location());

    return true;
}

bool run_transforms(CompilationState& state, Scope& scope, 
    std::span<DefinitionHeader* const> definitions) {

    for (auto header: definitions)
        if (header->body_)
            if (!run_transforms(state, scope, **header->body_))
                return false;

    return true;
}

} // namespace Maps
