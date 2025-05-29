#ifndef __REPL_IMPLEMENTATION_HH
#define __REPL_IMPLEMENTATION_HH

#include "../repl.hh"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <istream>
#include <array>
#include <utility>

#include "mapsc/compilation_state.hh"
#include "mapsc/ast/scope.hh"

#include "mapsci/jit_manager.hh"
#include "mapsc/parser/layer1.hh"

namespace Maps {

class REPL {
public:
    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream, 
        const REPL_Options& options);
    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream);

    bool run();

private:
    using DebugPrintSeparators = std::pair<std::string_view, std::string_view>; 

    static constexpr std::array<DebugPrintSeparators, REPL_STAGE_COUNT> debug_print_separators {
        DebugPrintSeparators{
            "------------------------- layer1 -------------------------", 
            "----------------------- layer1 end -----------------------"
        }, {
            "------------------ type name resolution ------------------", 
            "---------------- type name resolution end ----------------"
        }, {
            "-------------------- name resolution ---------------------", 
            "------------------ name resolution end -------------------"
        }, {
            "------------------------ layer2 --------------------------", 
            "---------------------- layer2-end ------------------------"
        }, {
            "-------------------- transform stage ---------------------", 
            "------------------ transform stage end -------------------"
        }, {
            "------------------- add repl wrapper ---------------------", 
            "----------------- add repl wrapper end -------------------"
        }, {
            "-------------------------- ir ----------------------------", 
            "------------------------ ir end --------------------------"
        }, {
            "", 
            ""
        }
    };

    bool save_history();
    std::optional<std::string> get_input();
    
    void debug_print(REPL_Stage stage, const RT_Scope& scope, 
        std::span<const Definition* const> extra_definitions);
    void debug_print(REPL_Stage stage, const RT_Scope& scope, const Definition*);
    void debug_print(REPL_Stage stage, const RT_Scope& scope);
    void debug_print(REPL_Stage stage, const llvm::Module& module);


    void run_command(CompilationState& state, const std::string& command);

    std::optional<Definition*> create_repl_wrapper(CompilationState& state, 
        RT_Definition* top_level_definition);
    bool compile_and_run(std::unique_ptr<llvm::Module> module_, const std::string& entry_point);
    
    bool run_compilation_pipeline(CompilationState& state, RT_Scope& global_scope, 
        std::istream& istream);
    std::string eval_type(std::istream& input_stream);
    
    Layer1Result run_layer1(CompilationState& state, 
        RT_Scope& global_scope, std::istream& source);
    bool run_layer2(CompilationState& state, 
        std::vector<Expression*> unparsed_termed_expressions);

    bool run_transforms(CompilationState& state, 
        RT_Scope& scope, std::optional<RT_Definition* const> definition);

    bool insert_global_cleanup(CompilationState& state, 
        RT_Scope& scope, RT_Definition& entry_point);

    std::unique_ptr<llvm::Module> run_ir_gen(CompilationState& state, 
        Definition* definition);

    // ----- PRIVATE FIELDS -----
    bool running_ = true;

    llvm::LLVMContext* context_;
    JIT_Manager* jit_;
    llvm::raw_ostream* error_stream_;
    REPL_Options options_ = {};
    ReverseParser reverse_parser_;
};

} // namespace Maps
    
#endif