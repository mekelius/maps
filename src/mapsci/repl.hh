#ifndef __REPL_HH
#define __REPL_HH

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <istream>

#include "mapsc/process_source.hh"

#include "mapsci/jit_manager.hh"


class REPL {
public:
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
        
        bool eval = true;

        bool ignore_errors = false;
        Stage stop_after = Stage::done;

        bool save_history = false;
        std::filesystem::path history_file_path;
        bool quit_on_error = false;
    };

    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream, Options options);
    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream);

    bool run();
    
private:
    std::optional<std::string> get_input();
    bool save_history();
    void eval(std::unique_ptr<llvm::Module> module_);
    std::string parse_type(std::istream& input_stream);
    void run_command(const std::string& command);

    void update_parse_options();

    bool running_ = true;

    llvm::LLVMContext* context_;
    JIT_Manager* jit_;
    llvm::raw_ostream* error_stream_;
    Options options_ = {};
    Maps::ProcessSourceOptions parse_options_ = {};

    // std::unique_ptr<Mapsc::Pragmas> pragmas_{};
};
    
#endif