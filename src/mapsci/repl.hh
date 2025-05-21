#ifndef __REPL_HH
#define __REPL_HH

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <istream>

#include "mapsc/process_source.hh"
#include "mapsc/compilation_state.hh"

#include "mapsci/jit_manager.hh"
#include "mapsc/parser/parser_layer1.hh"

class REPL {
public:
    static constexpr std::string_view DEFAULT_MODULE_NAME = "ir_module";
    static constexpr std::string_view DEFAULT_PROMPT = "mapsci> ";
    static constexpr std::string_view DEFAULT_REPL_WRAPPER_NAME = "mapsci_repl_wrapper";

    enum class Stage {
        layer1,
        layer2,
        layer3,
        ir,
        done
    };

    struct Options {
        bool print_layer1 = false;
        bool print_layer2 = false;
        bool print_layer3 = false;
        bool print_ir = false;
        bool print_compilation_state = false;
        
        bool eval = true;

        bool ignore_errors = false;
        Stage stop_after = Stage::done;
        bool quit_on_error = false;

        bool save_history = true;
        std::filesystem::path history_file_path;

        Maps::CompilationState::Options compiler_options{};

        std::string module_name = std::string{DEFAULT_MODULE_NAME};
        std::string prompt = std::string{DEFAULT_PROMPT};
        std::string repl_wrapper_name = std::string{DEFAULT_REPL_WRAPPER_NAME};
    };

    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream, Options options);
    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream);

    bool run();

private:
    bool save_history();
    std::optional<std::string> get_input();
    
    void run_command(Maps::CompilationState& state, const std::string& command);

    std::optional<Maps::Definition*> create_repl_wrapper(Maps::CompilationState& state, 
        Maps::RT_Definition* top_level_definition);
    bool compile_and_run(std::unique_ptr<llvm::Module> module_, const std::string& entry_point);
    
    bool run_compilation_pipeline(Maps::CompilationState& state, Maps::RT_Scope& global_scope, 
        std::istream& istream);
    std::string eval_type(std::istream& input_stream);
    
    Maps::ParserLayer1::Result run_layer1(Maps::CompilationState& state, 
        Maps::RT_Scope& global_scope, std::istream& source);
    bool run_layer2(Maps::CompilationState& state, 
        std::vector<Maps::Expression*> unparsed_termed_expressions);

    bool run_type_checks_and_concretize(Maps::CompilationState& state, 
        Maps::RT_Scope& scopes, Maps::RT_Definition* const definition);

    std::unique_ptr<llvm::Module> run_ir_gen(Maps::CompilationState& state, 
        Maps::Definition* definition);

    // ----- PRIVATE FIELDS -----
    bool running_ = true;

    llvm::LLVMContext* context_;
    JIT_Manager* jit_;
    llvm::raw_ostream* error_stream_;
    Options options_ = {};
};
    
#endif