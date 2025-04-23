#ifndef __REPL_HH
#define __REPL_HH

#include "llvm/Support/raw_os_ostream.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"

#include "lang/pragmas.hh"
#include "lang/ast.hh"

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
    struct Options {
        bool print_reverse_parse = false;
        bool print_ir = false;
        bool eval = true;
    };

    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream, Options options);
    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream);

    void run();
    
private:    
    void eval(const AST::AST& ast);
    void run_command(const std::string& command);

    bool running_ = true;
    bool is_good_ = true;

    llvm::LLVMContext* context_;
    JIT_Manager* jit_;
    llvm::raw_ostream* error_stream_;
    Options options_ = {};
    std::unique_ptr<Pragma::Pragmas> pragmas_{};
};
    
#endif