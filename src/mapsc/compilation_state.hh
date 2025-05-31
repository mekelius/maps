#ifndef __COMPILATION_STATE_HH
#define __COMPILATION_STATE_HH

#include <memory>
#include <tuple>
#include <variant>

#include "mapsc/pragma.hh"
#include "mapsc/builtins.hh"

#include "mapsc/types/type_store.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/scope.hh"


namespace Maps {

class CompilationState {
public:
    struct Options {
    };

    struct SpecialDefinitions {
        Operator* unary_minus;
        Operator* binary_minus;
        DefinitionHeader* print_String;
        DefinitionHeader* print_MutString;
    };

    static std::tuple<CompilationState, std::unique_ptr<Scope>, std::unique_ptr<TypeStore>> 
        create_test_state();

    static std::tuple<CompilationState, std::unique_ptr<TypeStore>> 
        create_test_state_with_builtins();

    CompilationState(const Scope* builtins, TypeStore* types, 
        SpecialDefinitions specials = {&unary_minus_Int, &binary_minus_Int, &print_String, &print_MutString});

    CompilationState(const Scope* builtins, TypeStore* types, 
        Options compiler_options,
        SpecialDefinitions specials = {&unary_minus_Int, &binary_minus_Int, &print_String, &print_MutString});

    // copy constructor
    CompilationState(const CompilationState&) = default;
    // copy assignment operator
    CompilationState& operator=(const CompilationState&) = default;
    ~CompilationState() = default;

    Options compiler_options_{};
    std::shared_ptr<AST_Store> ast_store_ = std::make_shared<AST_Store>();
    PragmaStore pragmas_ = {};
    
    TypeStore* types_;
    const Scope* builtins_;
    SpecialDefinitions special_definitions_ = 
        SpecialDefinitions{&unary_minus_Int, &binary_minus_Int, &print_String, &print_MutString};
};

} // namespace Maps

#endif