#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"

#include "llvm/Support/raw_os_ostream.h"

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"

#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

#include "logging.hh"
#include "lang/pragmas.hh"
#include "parsing/full_parse.hh"
#include "ir/ir_generator.hh"
#include "lang/reverse_parse.hh"

using std::unique_ptr, std::make_unique;
using Logging::LogLevel;

constexpr std::string_view USAGE = "USAGE: testci [source_file...]";
constexpr std::string_view DEFAULT_MODULE_NAME = "interpreted";
constexpr std::string_view PROMPT = "mapsci> ";

class JIT_Manager {
public:
    JIT_Manager(llvm::orc::ThreadSafeContext* context, llvm::raw_ostream* error_stream)
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

    bool compile_and_run(const std::string& name, std::unique_ptr<llvm::Module> module) {
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
    
        // excecute the compiled function
        auto *f1_ptr = f1_sym->toPtr<void(*)()>();
        f1_ptr();

        return true;
    }

    // resets everything in the jitDYlib
    void reset() {
        auto error = jit_->getMainJITDylib().clear();
        if (error) {
            *error_stream_ << error;
            error_stream_->flush();
        }
    }

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

    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream, Options options)
    : context_(context), jit_(jit), error_stream_(error_stream), options_(options) {}

    REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream)
    : context_(context), jit_(jit), error_stream_(error_stream) {}


    void run() {    
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
            
            unique_ptr<AST::AST> ast;
            std::stringstream input_s{input};
            std::tie(ast, pragmas_) = parse_source(input_s, true);

            if (options_.print_reverse_parse) {
                std::cout << "parsed into:\n";
                reverse_parse(*ast, std::cout);           
                std::cout << "\n" << std::endl;
            }

            eval(*ast);
        }
    }

    void eval(const AST::AST& ast) {
        jit_->reset();

        unique_ptr<llvm::Module> module_ = make_unique<llvm::Module>(DEFAULT_MODULE_NAME, *context_);
        unique_ptr<IR::IR_Generator> generator = make_unique<IR::IR_Generator>(context_, module_.get(), error_stream_);

        insert_builtins(*generator);
        generator->repl_run(ast, pragmas_.get());

        if (options_.print_ir) {
            std::cerr << "---IR DUMP---:\n\n";
            module_->dump();
            std::cerr << "\n---IR END---\n";
        }

        if (options_.eval) {
            jit_->compile_and_run(static_cast<std::string>(REPL_WRAPPER_NAME), std::move(module_));
        }
    }

    void run_command(const std::string& command) {
        if (command == ":q"     || 
            command == ":quit"  || 
            command == ":e"     || 
            command == ":exit"  || 
            command == ":c"     ||
            command == ":close"     
        ) running_ = false;
}
    
private:
    bool running_ = true;
    bool is_good_ = true;

    llvm::LLVMContext* context_;
    JIT_Manager* jit_;
    llvm::raw_ostream* error_stream_;
    Options options_ = {};
    std::unique_ptr<Pragma::Pragmas> pragmas_{};
};

int main(int argc, char* argv[]) {
    std::vector<std::string> args = {argv + 1, argc + argv};
    std::vector<std::string> source_filenames{};

    REPL::Options repl_options;
    repl_options.eval = true;
    repl_options.print_ir = true;
    repl_options.print_reverse_parse = true;

    for (std::string arg: args) {
        if (arg == "-t" || arg == "--tokens") {
            // lexer_ostream = &std::cout;

        } else if (arg == "--parser-debug") {
            Logging::Settings::set_loglevel(LogLevel::debug);
            
        } else if (arg == "-q" || arg == "--quiet") {
            Logging::Settings::set_loglevel(LogLevel::quiet);

        } else if (arg == "-e" || arg == "--everything") {
            Logging::Settings::set_loglevel(LogLevel::everything);

        } else {
            source_filenames.push_back(arg);
        }
    }

    Logging::init_logging(&std::cerr);
    llvm::raw_os_ostream error_stream{std::cerr};

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // llvm::InitializeNativeTarget();
    // llvm::InitializeNativeTargetAsmPrinter();
    // llvm::InitializeNativeTargetAsmParser();

        
    auto ts_context = make_unique<llvm::orc::ThreadSafeContext>(make_unique<llvm::LLVMContext>());
    JIT_Manager jit{ts_context.get(), &error_stream};

    if (!jit.is_good) {
        return EXIT_FAILURE;
    }

    REPL{&jit, ts_context->getContext(), &error_stream, repl_options}.run();
    
    return EXIT_SUCCESS;
}