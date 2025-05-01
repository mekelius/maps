#include "repl.hh"

#include <optional>
#include <memory>
#include <iostream>
#include <tuple>
#include <fstream>

#include "readline.h"
#include "history.h"

#include "llvm/Support/raw_os_ostream.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"

// ---------------------------------------------------

#include "logging.hh"

#include "ir/ir_generator.hh"

#include "parser/name_resolution.hh"
#include "parser/reverse_parse.hh"
#include "parser/parser_layer2.hh"
#include "parser/parser_layer1.hh"
#include "parser/full_parse.hh"
#include "parser/lexer.hh"

using std::optional, std::nullopt;
using std::unique_ptr, std::make_unique, std::make_optional, std::tuple;

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
: context_(context), jit_(jit), error_stream_(error_stream), options_(options) {
    update_parse_options();

    if (options_.save_history) {
        if (options_.history_file_path.empty()) {
            std::cout << "no history file given" << std::endl;
            return;
        }

        if (!std::filesystem::exists(options_.history_file_path))
            std::ofstream file{options_.history_file_path};

        // read history returns errno on failed read, but we also have to check that
        // errno wasn't 0 
        if (read_history(options_.history_file_path.c_str()) == errno && errno)
            std::cout << "ERROR: reading history failed" << std::endl;
    }
}

REPL::REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream)
: context_(context), jit_(jit), error_stream_(error_stream) {
    update_parse_options();
}

void REPL::run() {    
    while (running_) {
        char* line = readline(PROMPT.cbegin());

        if (!line) {
            std::cout << std::endl;
            break;
        }
        
        std::string input{line};
        
        if (input.empty()) {
            free(line);
            continue;
        }
        
        add_history(line);
        free(line);
        save_history();
    
        if (input.at(0) == ':') {
            run_command(input);
            continue;
        }
        
        std::stringstream input_s{input};

        auto [success, ast, pragmas] = parse_source(input_s, parse_options_, std::cerr);
  
        if (!success && options_.stop_on_error)
            continue;

        if (options_.stop_after == Stage::layer2)
            continue;

        if (options_.print_reverse_parse) {
            std::cerr << "parsed into:\n";
            reverse_parse(*ast, std::cout);
            std::cerr << "\n" << std::endl;
        }

        if (ast->is_valid)
            eval(*ast, *pragmas);
    }
}

bool REPL::save_history() {
    if (!options_.save_history || options_.history_file_path.empty()) 
        return true;

    if (write_history(options_.history_file_path.c_str()) == errno && errno) {
        std::cout << "ERROR: writing history failed" << std::endl;
        return false;
    }

    return true;
}
    
void REPL::eval(const Maps::AST& ast, Maps::Pragmas& pragmas) {
    jit_->reset();

    unique_ptr<llvm::Module> module_ = make_unique<llvm::Module>(DEFAULT_MODULE_NAME, *context_);
    unique_ptr<IR::IR_Generator> generator = make_unique<IR::IR_Generator>(context_, module_.get(), 
        ast, pragmas, error_stream_);

    insert_builtins(*generator);
    bool success = generator->repl_run();

    if (options_.print_ir) {
        std::cerr << "---IR DUMP---:\n\n";
        module_->dump();
        std::cerr << "\n---IR END---\n";
    }

    if (!success && options_.stop_on_error)
        return;

    if (options_.stop_after == Stage::ir)
        return;

    if (options_.eval) {
        jit_->compile_and_run(static_cast<std::string>(IR::REPL_WRAPPER_NAME), std::move(module_));
        std::cout << std::endl;
    }
}

std::string REPL::parse_type(std::istream& input_stream) {
    auto [success, ast, pragmas] = parse_source(input_stream, parse_options_, std::cout);
    
    if (!success && options_.stop_on_error) {
        std::cout << "ERROR: parsing type failed" << std::endl;
        return "";
    }

    auto type = ast->root_->get_type();

    return type->to_string();
}
    
void REPL::run_command(const std::string& input) {
    std::stringstream input_stream{input};
    std::string command;
    std::getline(input_stream, command, ' ');

    if (command == ":q"     || 
        command == ":quit"  || 
        command == ":e"     || 
        command == ":exit"  || 
        command == ":c"     ||
        command == ":close"   
    ) {
        running_ = false;
        return;
    }
    
    if (command == ":stop_after") {
        std::string arg;
        std::getline(input_stream, arg, ' ');

        if (arg == "layer1") {
            options_.stop_after = Stage::layer1;
            update_parse_options();
            return;
        }

        if (command == "layer2") {
            options_.stop_after = Stage::layer2;
            update_parse_options();
            return;
        } 
        
        if (command == "ir") {
            options_.stop_after = Stage::ir;
            update_parse_options();
            return;
        }
        
        if (command == "done" || command == "eval") {
            options_.stop_after = Stage::done;
            update_parse_options();
            return;
        }

        return;
    } 
    
    if (command == ":toggle") { 
        std::string arg;
        std::getline(input_stream, arg, ' ');

        if (arg == "eval") {
            options_.eval = !options_.eval;
            std::cout << "eval " << (options_.eval ? "on" : "off") << std::endl;
            return;
        }

        if (arg == "print_layer1") {
            options_.print_layer1 = !options_.print_layer1;
            update_parse_options();
            std::cout << "print_layer1 " << (options_.eval ? "on" : "off") << std::endl;
            return;
        }

        if (arg == "print_layer2") {
            options_.print_layer2 = !options_.print_layer2;
            update_parse_options();
            std::cout << "print_layer2 " << (options_.eval ? "on" : "off") << std::endl;
            return;
        }
        
        if (arg == "print_ir") {
            options_.print_ir = !options_.print_ir;
            std::cout << "print_ir " << (options_.eval ? "on" : "off") << std::endl;
            return;
        }

        if (arg == "stop_on_error") {
            options_.stop_on_error = !options_.stop_on_error;
            update_parse_options();
            std::cout << "stop_on_error " << (options_.eval ? "on" : "off") << std::endl;
            return;
        }

        std::cout << "\"" << arg << "\" is not a toggle" << std::endl;
        return;
    }
    
    if (command == ":t") {
        std::cout << parse_type(input_stream) << std::endl;
        return;
    }
    
    std::cout << "\"" << command << "\" is not a command" << std::endl;
    return;
}

void REPL::update_parse_options() {
    parse_options_ = {
        options_.print_layer1,
        options_.print_layer2,
        options_.stop_on_error,
        ParseStage::done,
        true
    };

    if (options_.stop_after == Stage::layer1) {
        parse_options_.stop_after = ParseStage::layer1;
        return;
    }

    if (options_.stop_after == Stage::layer2) {
        parse_options_.stop_after = ParseStage::layer2;
        return;
    }
}