#include "compilation_state.hh"
#include "mapsc/types/type_store.hh"

using std::tuple, std::unique_ptr, std::make_unique;

namespace Maps {

tuple<CompilationState, unique_ptr<const Scope>, unique_ptr<TypeStore>> CompilationState::create_test_state() {
    auto types = make_unique<TypeStore>();
    auto builtins = make_unique<const Scope>();
    
    return tuple{CompilationState{builtins.get(), types.get()}, std::move(builtins), std::move(types)};
}

CompilationState::CompilationState(const Scope* builtins, TypeStore* types, 
    SpecialCallables special_callables)
:types_(types), builtins_(builtins), special_callables_(special_callables) {
}

bool CompilationState::set_entry_point(Callable* entrypoint) {
    entry_point_ = entrypoint;
    return true;
}

bool CompilationState::set_entry_point(std::string name) {
    auto entry_point = globals_->get_identifier(name);

    if (!entry_point)
        return false;

    entry_point_ = entry_point;
    return true;
}


} // namespace Maps