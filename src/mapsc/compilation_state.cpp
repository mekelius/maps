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
:CompilationState(builtins, types, {}, special_callables) {}

CompilationState::CompilationState(const Scope* builtins, TypeStore* types, 
    Options compiler_options, SpecialCallables special_callables)
:compiler_options_(compiler_options), 
 types_(types), 
 builtins_(builtins), 
 special_callables_(special_callables) {}

bool CompilationState::empty() const {
    if (!globals_.empty())
        return false;

    if (entry_point_)
        return false;

    return true;
}

bool CompilationState::set_entry_point(Callable* entrypoint) {
    entry_point_ = entrypoint;
    return true;
}

bool CompilationState::set_entry_point(std::string name) {
    auto entry_point = globals_.get_identifier(name);

    if (!entry_point)
        return false;

    entry_point_ = entry_point;
    return true;
}

void CompilationState::dump(std::ostream& stream) const {
    stream << "----- Compilation state -----\n\n";
    
    stream << (is_valid ? "valid" : "invalid") << "\n"; 
    stream << "entry point: ";
    if (!entry_point_) {
        stream << "none\n";
    } else {
        stream << (*entry_point_)->name_ << "\n";
    }

    stream << "global identifiers:\n";

    for (auto [name, callable]: globals_.identifiers_in_order_) {
        stream << "  " << name << "\n";
    }

    stream << "\n-----------------------------\n" << std::endl;
}

} // namespace Maps