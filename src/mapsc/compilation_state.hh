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
        CT_Operator* unary_minus;
        CT_Operator* binary_minus;
        CT_Definition* print_String;
        CT_Definition* print_MutString;
    };

    static std::tuple<CompilationState, std::unique_ptr<const CT_Scope>, std::unique_ptr<TypeStore>> 
        create_test_state();

    static std::tuple<CompilationState, std::unique_ptr<TypeStore>> 
        create_test_state_with_builtins();

    CompilationState(const CT_Scope* builtins, TypeStore* types, 
        SpecialDefinitions specials = {&unary_minus_Int, &binary_minus_Int, &print_String, &print_MutString});

    CompilationState(const CT_Scope* builtins, TypeStore* types, 
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
    const CT_Scope* builtins_;
    SpecialDefinitions special_definitions_ = 
        SpecialDefinitions{&unary_minus_Int, &binary_minus_Int, &print_String, &print_MutString};
};

} // namespace Maps

#endif