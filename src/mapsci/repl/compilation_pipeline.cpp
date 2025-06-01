#include "implementation.hh"

#include <sstream>
#include <utility>
#include <variant>
#include <optional>
#include <memory>
#include <iostream>

#include "readline.h"

#include "mapsc/builtins.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/transform_stage.hh"

#include "mapsc/types/type.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/let_definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/call_expression.hh"

#include "mapsc/parser/layer1.hh"
#include "mapsc/parser/layer2.hh"

#include "mapsc/procedures/name_resolution.hh"
#include "mapsc/procedures/cleanup.hh"
#include "mapsc/procedures/concretize.hh"

#include "mapsc/llvm_ir_gen/ir_generator.hh"
#include "mapsc/llvm_ir_gen/ir_builtins.hh"

using std::unique_ptr, std::make_unique, std::make_optional, std::tuple, std::optional, std::nullopt;

namespace Maps {

optional<DefinitionBody*> REPL::create_repl_wrapper(CompilationState& state, Scope& global_scope, 
    DefinitionBody* top_level_definition) {
    
    std::cout << "Creating REPL wrapper...\n";

    std::optional<Expression*> eval_and_print;
    auto location = NO_SOURCE_LOCATION;
    auto top_level_type = top_level_definition->get_type();

    if (top_level_type->is_function()) {
        auto run_eval = create_call(state, top_level_definition, {}, location);

        if (!run_eval) {
            std::cout << "creating REPL wrapper failed: could not create top-level call" 
                      << std::endl;
            return nullopt;
        }

        // try string first
        eval_and_print = create_call(state, state.special_definitions_.print_String, 
            {*run_eval}, location);

        if (!eval_and_print)
            eval_and_print = create_call(state, state.special_definitions_.print_MutString, 
                {*run_eval}, location);


    } else {
        eval_and_print = std::visit(overloaded{
            [&state, location](Expression* expression)->optional<Expression*> {
                return create_call(state, state.special_definitions_.print_String,
                    { expression }, location);
            },
            [&state, location](Statement* statement)->optional<Expression*> {
                switch (statement->statement_type) {
                    case StatementType::expression_statement:
                    case StatementType::return_:
                        return create_call(state, state.special_definitions_.print_String,
                            {statement->get_value<Expression*>()}, location);
                    default:
                        assert(false && "not implemented");
                }
            },
            [](auto)->optional<Expression*> {
                assert(false && "not implemented");
            }
        }, top_level_definition->body());

        if (!eval_and_print)
            eval_and_print = create_call(state, state.special_definitions_.print_MutString,
                {std::get<Expression*>(top_level_definition->body())}, 
                location);
    }

    if (!eval_and_print && top_level_type->is_pure()) {
        std::cout << "creating REPL wrapper failed" << std::endl;
        return nullopt;
    }

    if (!eval_and_print && top_level_type->is_impure()) {
        auto eval = create_call(state, top_level_definition, {}, location);

        if (!eval) {
            std::cout << "creating REPL wrapper failed" << std::endl;
            return nullopt;
        }

        return create_let_definition(*state.ast_store_, &global_scope, options_.repl_wrapper_name, 
            *eval, true, location);
    }

    auto definition = create_let_definition(*state.ast_store_, &global_scope, options_.repl_wrapper_name, 
        *eval_and_print, true, location);

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
    Scope& global_scope, std::istream& source) {

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

    debug_print(REPL_Stage::layer1, global_scope, (*top_level_definition)->header_);

    if (options_.stop_after == REPL_Stage::layer1)
        return true;


    // ------------------------------ NAME RESOLUTION -----------------------------------

    if (!resolve_identifiers(state, global_scope, unresolved_type_identifiers) && 
            !options_.ignore_errors)
        return false;

    debug_print(REPL_Stage::type_name_resolution, global_scope, (*top_level_definition)->header_);

    // if (!la(possible_binding_type_declarations) && !options_.ignore_errors)
    //     return false;

    if (!resolve_identifiers(state, global_scope, unresolved_identifiers) && 
            !options_.ignore_errors)
        return false;

    debug_print(REPL_Stage::name_resolution, global_scope, (*top_level_definition)->header_);


    // ----------------------------------- LAYER2 ----------------------------------------

    if (!run_layer2(state, unparsed_termed_expressions) && !options_.ignore_errors)
        return false;

    debug_print(REPL_Stage::layer2, global_scope, (*top_level_definition)->header_);

    if (options_.stop_after == REPL_Stage::layer2)
        return true;


    // --------------------------------- TRANSFORM STAGE --------------------------------------

    if (!run_transforms(state, global_scope, top_level_definition) && !options_.ignore_errors) {
        std::cout << "Transform stage failed" << std::endl;
        return false;
    }

    debug_print(REPL_Stage::transform_stage, global_scope, (*top_level_definition)->header_);


    // ----------------------------- CREATE REPL WRAPPER ---------------------------------

    if (!top_level_definition || ((*top_level_definition)->get_type()->is_pure() && (*top_level_definition)->get_type()->is_voidish())) {
        std::cout << "Top level definition doesn't produce a value or doesn't exist" << std::endl;
        return true;
    }

    auto repl_wrapper = create_repl_wrapper(state, global_scope, *top_level_definition);

    if (!repl_wrapper) {
        debug_print(REPL_Stage::pre_ir, global_scope, (*top_level_definition)->header_);
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
        std::array<const DefinitionHeader*, 2>{(*top_level_definition)->header_, (*repl_wrapper)->header_});

    if (options_.stop_after == REPL_Stage::pre_ir || !options_.eval)
        return true;


    // ------------------------------------ IR GEN ---------------------------------------

    unique_ptr<llvm::Module> module_ = make_unique<llvm::Module>(options_.module_name, *context_);
    LLVM_IR::IR_Generator generator{context_, module_.get(), &state, error_stream_};

    if (!LLVM_IR::insert_builtins(generator)) {
        std::cout << "Inserting IR builtins failed\n";
        return false;
    }

    bool ir_success = generator.run({std::array{&global_scope}}, 
        std::array{(*top_level_definition)->header_, (*repl_wrapper)->header_});

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


Layer1Result REPL::run_layer1(CompilationState& state, Scope& global_scope, std::istream& source) {
    return run_layer1_eval(state, global_scope, source);
}

bool REPL::run_layer2(CompilationState& state, std::vector<Expression*> unparsed_termed_expressions) {
    return Maps::run_layer2(state, unparsed_termed_expressions);
}

bool REPL::run_transforms(CompilationState& state, 
    Scope& scope, optional<DefinitionBody* const> top_level_definition) {

    for (auto definition: scope) {
        if (*definition->body_)
            if (!Maps::concretize(state, **definition->body_))
                return false;
    }

    if (!Maps::run_transforms(state, scope, scope.identifiers_in_order_))
        return false;
        
    if (!top_level_definition)
        return true;

    if (!Maps::concretize(state, **top_level_definition))
        return false;

    return true;
}

bool REPL::insert_global_cleanup(Maps::CompilationState& state, 
    Maps::Scope& scope, Maps::DefinitionBody& entry_point) {

    return Maps::insert_cleanup(state, scope, entry_point);
}

} // namespace Maps
