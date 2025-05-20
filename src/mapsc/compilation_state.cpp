#include "compilation_state.hh"
#include "mapsc/types/type_store.hh"

using std::tuple, std::unique_ptr, std::make_unique;

namespace Maps {

tuple<CompilationState, unique_ptr<const CT_Scope>, unique_ptr<TypeStore>> CompilationState::create_test_state() {
    auto types = make_unique<TypeStore>();
    auto builtins = make_unique<const CT_Scope>();
    
    return tuple{CompilationState{builtins.get(), types.get()}, std::move(builtins), std::move(types)};
}

CompilationState::CompilationState(const CT_Scope* builtins, TypeStore* types, 
    SpecialDefinitions special_definitions)
:CompilationState(builtins, types, {}, special_definitions) {}

CompilationState::CompilationState(const CT_Scope* builtins, TypeStore* types, 
    Options compiler_options, SpecialDefinitions special_definitions)
:compiler_options_(compiler_options), 
 types_(types), 
 builtins_(builtins), 
 special_definitions_(special_definitions) {}

bool CompilationState::empty() const {
    if (!globals_.empty())
        return false;

    if (entry_point_)
        return false;

    return true;
}

bool CompilationState::set_entry_point(RT_Definition* entrypoint) {
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
        stream << (*entry_point_)->name() << "\n";
    }

    stream << "global identifiers:\n";

    for (auto [name, _]: globals_.identifiers_in_order_) {
        stream << "  " << name << "\n";
    }

    stream << "\n-----------------------------\n" << std::endl;
}

} // namespace Maps