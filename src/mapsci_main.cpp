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

    // llvm::orc::ResourceTracker* compile_and_run_with_ro(llvm::Module& module) {
    //     auto resource_tracker = jit_->getMainJITDylib().createResourceTracker();
    // }

    template<typename T>
    std::optional<T> compile_and_run(const std::string& name, std::unique_ptr<llvm::Module> module) {
        if (auto err = jit_->addIRModule(llvm::orc::ThreadSafeModule{std::move(module), *context_})) {
            *error_stream_ << err << '\n';
            return std::nullopt;
        }
    
        auto f1_sym = jit_->lookup(name);
        if (!f1_sym) {
            *error_stream_ << f1_sym.takeError() << '\n';
            return std::nullopt;
        }
    
        auto *f1_ptr = f1_sym->toPtr<T(*)()>();
        
        return f1_ptr();
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
            std::tie(ast, pragmas_) = parse_source(input_s);

            // std::cout << eval(*ast);
        }
    }

    void eval(const AST::AST& ast) {
        unique_ptr<llvm::Module> module_ = make_unique<llvm::Module>(DEFAULT_MODULE_NAME, *context_);
        IR::IR_Generator{context_, module_.get(), error_stream_}.repl_run(ast, pragmas_.get());

        module_->dump();

        jit_->compile_and_run<std::string>(ast.entry_point_->name, std::move(module_));
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

    // InitializeAllTargetInfos();
    // InitializeAllTargets();
    // InitializeAllTargetMCs();
    // InitializeAllAsmParsers();
    // InitializeAllAsmPrinters();

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

        
    auto ts_context = make_unique<llvm::orc::ThreadSafeContext>(make_unique<llvm::LLVMContext>());
    JIT_Manager jit{ts_context.get(), &error_stream};

    if (!jit.is_good) {
        return EXIT_FAILURE;
    }

    REPL{&jit, ts_context->getContext(), &error_stream}.run();
    
    return EXIT_SUCCESS;
}