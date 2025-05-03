#ifndef __REPL_HH
#define __REPL_HH

#include <filesystem>

#include "llvm/Support/raw_os_ostream.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"

#include "mapsc/parser/full_parse.hh"
#include "mapsc/pragma.hh"
#include "mapsc/ast/ast.hh"

class JIT_Manager {
public:
    JIT_Manager(llvm::orc::ThreadSafeContext* context, llvm::raw_ostream* error_stream);

    // llvm::orc::ResourceTracker* compile_and_run_with_rt(llvm::Module& module) {
    //     auto resource_tracker = jit_->getMainJITDylib().createResourceTracker();
    // }

    bool compile_and_run(const std::string& name, std::unique_ptr<llvm::Module> module);

    // resets everything in the jitDYlib
    void reset();

    bool is_good = true;
private:
    llvm::raw_ostream* error_stream_;
    llvm::orc::ThreadSafeContext* context_;
    std::unique_ptr<llvm::orc::LLJIT> jit_;
};

class REPL {
public:
    enum class Stage {
        layer1,
        layer2,
        ir,
        done
    };

    struct Options {
        bool print_layer1 = false;
        bool print_layer2 = false;
        bool print_reverse_parse = false;
        bool print_ir = false;
        
        bool eval = true;

        bool ignore_errors = false;
        Stage stop_after = Stage::done;

        bool save_history = false;
        std::filesystem::path history_file_path;
    };

    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream, Options options);
    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream);

    void run();
    
private:
    bool save_history();
    void eval(const Maps::AST_Store& ast, Maps::PragmaStore& pragmas);
    std::string parse_type(std::istream& input_stream);
    void run_command(const std::string& command);

    void update_parse_options();

    bool running_ = true;

    llvm::LLVMContext* context_;
    JIT_Manager* jit_;
    llvm::raw_ostream* error_stream_;
    Options options_ = {};
    ParseOptions parse_options_ = {};

    // std::unique_ptr<Mapsc::Pragmas> pragmas_{};
};
    
#endif