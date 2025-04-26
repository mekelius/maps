#include "repl.hh"

#include <optional>
#include <memory>
#include <iostream>

#include "llvm/Support/raw_os_ostream.h"

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"

#include "ir/ir_generator.hh"
#include "lang/reverse_parse.hh"

#include "logging.hh"
#include "parsing/full_parse.hh"


using std::optional, std::nullopt;
using std::unique_ptr, std::make_unique;

constexpr std::string_view DEFAULT_MODULE_NAME = "interpreted";
constexpr std::string_view PROMPT = "mapsci> ";

JIT_Manager::JIT_Manager(llvm::orc::ThreadSafeContext* context, llvm::raw_ostream* error_stream)
    : error_stream_(error_stream), context_(context) {

    auto jit = llvm::orc::LLJITBuilder().create();

    if (!jit) {
        *error_stream_ << jit.takeError() << '\n';
        is_good = false;
        return;
    }

    jit_ = std::move(*jit);
}

// llvm::orc::ResourceTracker* compile_and_run_with_rt(llvm::Module& module) {
//     auto resource_tracker = jit_->getMainJITDylib().createResourceTracker();
// }
    
bool JIT_Manager::compile_and_run(const std::string& name, std::unique_ptr<llvm::Module> module) {
    if (auto err = jit_->addIRModule(llvm::orc::ThreadSafeModule{std::move(module), *context_})) {
        *error_stream_ << err << '\n';
        error_stream_->flush();
        return false;
    }

    auto f1_sym = jit_->lookup(name);
    if (!f1_sym) {
        *error_stream_ << f1_sym.takeError() << '\n';
        error_stream_->flush();
        return false;
    }

    // execute the compiled function
    auto *f1_ptr = f1_sym->toPtr<void(*)()>();
    f1_ptr();

    return true;
}

// resets everything in the jitDYlib
void JIT_Manager::reset() {
    auto error = jit_->getMainJITDylib().clear();
    if (error) {
        *error_stream_ << error;
        error_stream_->flush();
    }
}
    
REPL::REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream, Options options)
: context_(context), jit_(jit), error_stream_(error_stream), options_(options) {}

REPL::REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream)
: context_(context), jit_(jit), error_stream_(error_stream) {}

    
void REPL::run() {    
    while (running_) {
        std::cout << PROMPT;

        std::string input;
        std::getline(std::cin, input);

        if (std::cin.eof()) {
            std::cout << std::endl;
            running_ = false;
        }

        if (input.empty())
            continue;
    
        if (input.at(0) == ':') {
            run_command(input);
            continue;
        }
        
        unique_ptr<Maps::AST> ast;
        std::stringstream input_s{input};
        auto result = parse_source(input_s, true);
        if (!result) {
            Logging::log_error("parsing failed");
            continue;
        }

        std::tie(ast, pragmas_) = std::move(*result);

        if (!ast->is_valid) {
            Logging::log_error("parsing failed");
            continue;
        }

        if (options_.print_reverse_parse) {
            std::cout << "parsed into:\n";
            reverse_parse(*ast, std::cout);           
            std::cout << "\n" << std::endl;
        }

        if (ast->is_valid)
            eval(*ast);
    }
}
    
void REPL::eval(const Maps::AST& ast) {
    jit_->reset();

    unique_ptr<llvm::Module> module_ = make_unique<llvm::Module>(DEFAULT_MODULE_NAME, *context_);
    unique_ptr<IR::IR_Generator> generator = make_unique<IR::IR_Generator>(context_, module_.get(), 
        ast, *pragmas_, error_stream_);

    insert_builtins(*generator);
    generator->repl_run();

    if (options_.print_ir) {
        std::cerr << "---IR DUMP---:\n\n";
        module_->dump();
        std::cerr << "\n---IR END---\n";
    }

    if (options_.eval) {
        jit_->compile_and_run(static_cast<std::string>(IR::REPL_WRAPPER_NAME), std::move(module_));
        std::cout << std::endl;
    }
}
    
void REPL::run_command(const std::string& command) {
    if (command == ":q"     || 
        command == ":quit"  || 
        command == ":e"     || 
        command == ":exit"  || 
        command == ":c"     ||
        command == ":close"   
    ) running_ = false;
}
           