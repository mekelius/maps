#include "compilation_state.hh"
#include "mapsc/types/type_store.hh"

using std::tuple, std::unique_ptr, std::make_unique;

namespace Maps {

tuple<CompilationState, unique_ptr<TypeStore>> 
    CompilationState::create_test_state() {
    
    auto types = make_unique<TypeStore>();
    auto builtins = make_unique<Scope>();
    
    return tuple{CompilationState{types.get()}, std::move(types)};
}

CompilationState::CompilationState(TypeStore* types, 
    SpecialDefinitions special_definitions)
:CompilationState(types, {}, special_definitions) {}

CompilationState::CompilationState(TypeStore* types, 
    Options compiler_options, SpecialDefinitions special_definitions)
:compiler_options_(compiler_options), 
 types_(types), 
 special_definitions_(special_definitions) {}

} // namespace Maps