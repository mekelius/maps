#ifndef __COMPILATION_STATE_HH
#define __COMPILATION_STATE_HH

#include <vector>
#include <memory>
#include <string>

#include "mapsc/pragma.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/ast/scope.hh"

namespace Maps {

class CompilationState {
public:
    static std::tuple<CompilationState, std::unique_ptr<const Scope>, std::unique_ptr<TypeStore>> 
        create_test_state();

    CompilationState(const Scope* builtins, TypeStore* types);

    [[nodiscard]] bool set_entry_point(Callable* entrypoint);
    [[nodiscard]] bool set_entry_point(std::string name);

    void declare_invalid() { is_valid = false; };

    bool is_valid = true;
    
    std::unique_ptr<Scope> globals_ = std::make_unique<Scope>();
    std::unique_ptr<AST_Store> ast_store_ = std::make_unique<AST_Store>();
    std::unique_ptr<PragmaStore> pragmas_ = std::make_unique<PragmaStore>();
    
    // container for top-level statements
    
    std::optional<Callable*> entry_point_ = std::nullopt;
    
    TypeStore* const types_;
    const Scope* const builtins_;

    // layer1 fills these with pointers to expressions that need work so that layer 2 doesn't
    // need to walk the tree to find them
    std::vector<Expression*> unresolved_identifiers_ = {};
    std::vector<Expression*> unresolved_type_identifiers_ = {};
    // these have to be dealt with before name resolution
    std::vector<Expression*> possible_binding_type_declarations_ = {};
    std::vector<Expression*> unparsed_termed_expressions_ = {};
};

} // namespace Maps

#endif