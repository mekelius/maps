#include "compilation_state.hh"
#include "mapsc/types/type_store.hh"

using std::tuple, std::unique_ptr, std::make_unique;

namespace Maps {

tuple<CompilationState, unique_ptr<Scope>, unique_ptr<TypeStore>> 
    CompilationState::create_test_state() {
    
    auto types = make_unique<TypeStore>();
    auto builtins = make_unique<Scope>();
    
    return tuple{CompilationState{builtins.get(), types.get()}, std::move(builtins), std::move(types)};
}

tuple<CompilationState, unique_ptr<TypeStore>> CompilationState::create_test_state_with_builtins() {
    auto types = make_unique<TypeStore>();
    
    return tuple{CompilationState{get_builtins(), types.get()}, std::move(types)};
}

CompilationState::CompilationState(const Scope* builtins, TypeStore* types, 
    SpecialDefinitions special_definitions)
:CompilationState(builtins, types, {}, special_definitions) {}

CompilationState::CompilationState(const Scope* builtins, TypeStore* types, 
    Options compiler_options, SpecialDefinitions special_definitions)
:compiler_options_(compiler_options), 
 types_(types), 
 builtins_(builtins), 
 special_definitions_(special_definitions) {}

} // namespace Maps