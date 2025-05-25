#include "implementation.hh"

#include <cerrno>
#include <sstream>
#include <string_view>
#include <utility>
#include <variant>
#include <cstdlib>
#include <optional>
#include <memory>
#include <iostream>
#include <fstream>

#include "readline.h"

#include "mapsc/builtins.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/procedures/reverse_parse.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/types/type.hh"
#include "mapsc/parser/layer1.hh"
#include "mapsc/parser/layer2.hh"
#include "mapsc/procedures/type_check.hh"
#include "mapsc/procedures/name_resolution.hh"
#include "mapsc/transform_stage.hh"
#include "mapsc/procedures/cleanup.hh"

#include "mapsc/llvm/ir_generator.hh"
#include "mapsc/llvm/ir_builtins.hh"

using std::unique_ptr, std::make_unique, std::make_optional, std::tuple, std::optional, std::nullopt;
using Maps::CompilationState, Maps::Layer1Result, Maps::run_layer1_eval, Maps::SimpleTypeChecker, 
    Maps::Definition, Maps::RT_Definition, Maps::CT_Definition, Maps::Expression,
    Maps::CT_Scope, Maps::RT_Scope, Maps::Scopes, Maps::ReverseParser,
    Maps::resolve_identifiers;

optional<Definition*> REPL::create_repl_wrapper(CompilationState& state, 
    RT_Definition* top_level_definition) {
    
    std::optional<Expression*> eval_and_print;
    auto location = NO_SOURCE_LOCATION;

    if (top_level_definition->get_type()->is_function()) {
        auto run_eval = Expression::call(state, top_level_definition, {}, location);

        if (!run_eval) {
            std::cout << "creating REPL wrapper failed: could not create top-level call" 
                      << std::endl;
            return nullopt;
        }

        eval_and_print = Expression::call(state, state.special_definitions_.print_String, 
            {*run_eval}, top_level_definition->location());

    } else {
        eval_and_print = Expression::call(state, state.special_definitions_.print_String,
            {std::get<Expression*>(top_level_definition->body())}, 
            location);
    }

    if (!eval_and_print) {
        std::cout << "creating REPL wrapper failed" << std::endl;
        return nullopt;
    }

    auto definition = state.ast_store_->allocate_definition(
        RT_Definition{options_.repl_wrapper_name, *eval_and_print, true, location});

    // SimpleTypeChecker{}.run(state, {}, std::array<RT_Definition* const, 1>{definition});

    return definition;    
}

bool REPL::compile_and_run(std::unique_ptr<llvm::Module> module_, const std::string& entry_point) {
    jit_->reset();
    jit_->compile_and_run(std::move(module_), entry_point);
    std::cout << std::endl;
    return true;
}

std::string REPL::eval_type(std::istream& input_stream) {
    assert(false && "not implemented");
    // Maps::TypeStore types{};

    // auto state = 
    //     process_source(Maps::get_builtins(), &types, input_stream,, std::cout);
    
    // if (!state->is_valid && !options_.ignore_errors) {
    //     std::cout << "ERROR: parsing type failed" << std::endl;
    //     return "";
    // }

    // auto type = (*state->entry_point_)->get_type();

    // return type->to_string();
}

bool REPL::run_compilation_pipeline(CompilationState& state, 
    RT_Scope& global_scope, std::istream& source) {

    // ---------------------------------- LAYER1 ----------------------------------------

    auto [ 
        layer1_success,
        top_level_definition,
        unresolved_identifiers,
        unresolved_type_identifiers,
        unparsed_termed_expressions,
        possible_binding_type_declarations
    ] = run_layer1(state, global_scope, source);

    if (!layer1_success && !options_.ignore_errors)
        return false;

    debug_print(REPL_Stage::layer1, global_scope, *top_level_definition);

    if (options_.stop_after == REPL_Stage::layer1)
        return true;


    // ------------------------------ NAME RESOLUTION -----------------------------------

    if (!resolve_identifiers(state, global_scope, unresolved_type_identifiers) && 
            !options_.ignore_errors)
        return false;

    debug_print(REPL_Stage::type_name_resolution, global_scope, *top_level_definition);

    // if (!la(possible_binding_type_declarations) && !options_.ignore_errors)
    //     return false;

    if (!resolve_identifiers(state, global_scope, unresolved_identifiers) && 
            !options_.ignore_errors)
        return false;

    debug_print(REPL_Stage::name_resolution, global_scope, *top_level_definition);


    // ----------------------------------- LAYER2 ----------------------------------------

    if (!run_layer2(state, unparsed_termed_expressions) && !options_.ignore_errors)
        return false;

    debug_print(REPL_Stage::layer2, global_scope, *top_level_definition);

    if (options_.stop_after == REPL_Stage::layer2)
        return true;

    if (!top_level_definition || (*top_level_definition)->get_type()->is_voidish())
        return true;


    // --------------------------------- TYPE CHECKS --------------------------------------

    if (!run_transforms(state, global_scope, top_level_definition))
        return false;

    debug_print(REPL_Stage::transform_stage, global_scope, *top_level_definition);


    // ----------------------------- CREATE REPL WRAPPER ---------------------------------

    if (!top_level_definition || (*top_level_definition)->get_type()->is_voidish())
        return true;

    auto repl_wrapper = create_repl_wrapper(state, *top_level_definition);

    if (!repl_wrapper) {
        debug_print(REPL_Stage::pre_ir, global_scope, *top_level_definition);
        std::cout << "ERROR: creating repl wrapper failed" << std::endl;
        if (!options_.ignore_errors)
            return false;
    }

    if (!insert_global_cleanup(state, 
        global_scope, **top_level_definition)) {

        std::cout << "ERROR: creating repl wrapper failed" << std::endl;
        if (!options_.ignore_errors)
            return false;
    }

    debug_print(REPL_Stage::pre_ir, global_scope, 
        std::array<const Definition*, 2>{*top_level_definition, *repl_wrapper});

    if (options_.stop_after == REPL_Stage::pre_ir || !options_.eval)
        return true;


    // ------------------------------------ IR GEN ---------------------------------------

    unique_ptr<llvm::Module> module_ = make_unique<llvm::Module>(options_.module_name, *context_);
    IR::IR_Generator generator{context_, module_.get(), &state, error_stream_};

    if (!IR::insert_builtins(generator)) {
        std::cout << "Inserting IR builtins failed\n";
        return false;
    }

    bool ir_success = generator.run({std::array<RT_Scope* const, 1>{&global_scope}}, 
        std::array<Definition*, 2>{*top_level_definition, *repl_wrapper});

    debug_print(REPL_Stage::ir, *module_);

    if (!ir_success && !options_.ignore_errors) {
        std::cout << "IR gen failed\n";
        return false;
    }

    if (options_.stop_after == REPL_Stage::ir || !options_.eval)
        return true;


    // -------------------------------- COMPILE AND RUN ---------------------------------

    std::cout << '\n';
    return compile_and_run(std::move(module_), options_.repl_wrapper_name);
}


Layer1Result REPL::run_layer1(CompilationState& state, RT_Scope& global_scope, std::istream& source) {
    return run_layer1_eval(state, global_scope, source);
}

bool REPL::run_layer2(CompilationState& state, std::vector<Expression*> unparsed_termed_expressions) {
    return Maps::run_layer2(state, unparsed_termed_expressions);
}

bool REPL::run_transforms(CompilationState& state, 
    RT_Scope& scope, optional<RT_Definition* const> top_level_definition) {

    Maps::run_transforms(state, scope, scope.identifiers_in_order_);
        

    if (top_level_definition) {
        Maps::run_transforms(state, scope, std::array<RT_Definition* const, 1>{*top_level_definition});
        // if (!SimpleTypeChecker{}.run(state, std::array<RT_Scope* const, 1>{&scope}, 
        //         std::array<RT_Definition* const, 1>{*top_level_definition}))
        //     return false;
    } else {
        // if (!SimpleTypeChecker{}.run(state, std::array<RT_Scope* const, 1>{&scope}, {}))
        //     return false;
    }

    return true;
}

bool REPL::insert_global_cleanup(Maps::CompilationState& state, 
    Maps::RT_Scope& scope, Maps::RT_Definition& entry_point) {

    return Maps::insert_cleanup(state, scope, entry_point);
}