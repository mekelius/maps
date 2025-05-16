#include "repl.hh"

#include <cerrno>
#include <sstream>
#include <string_view>
#include <utility>
#include <variant>
#include <cstdlib>
#include <optional>
#include <memory>
#include <iostream>
#include <fstream>

#include "readline.h"

#include "mapsc/builtins.hh"
#include "mapsc/ast/callable.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/process_source.hh"
#include "mapsc/procedures/reverse_parse.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/types/type.hh"

#include "mapsc/llvm/ir_generator.hh"


using std::optional, std::nullopt;
using std::unique_ptr, std::make_unique, std::make_optional, std::tuple;
using Maps::ProcessSourceOptions, Maps::ReverseParser, Maps::CompilationLayer;

constexpr std::string_view DEFAULT_MODULE_NAME = "interpreted";
constexpr std::string_view PROMPT = "mapsci> ";

REPL::REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream, Options options)
: context_(context), jit_(jit), error_stream_(error_stream), options_(options) {
    parse_options_.in_repl = true;
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

REPL::REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream) {
    REPL(jit, context, error_stream, {});
}

bool REPL::run() {    
    while (running_) {
        
        optional<std::string> input = get_input();

        if (!input)
            continue;

        if (input->at(0) == ':') {
            run_command(*input);
            continue;
        }

        std::stringstream input_s{*input};
        Maps::TypeStore types{};
        auto compilation_state = process_source(Maps::get_builtins(), &types, input_s, 
            parse_options_, std::cerr);
        
        if (!compilation_state->is_valid) {
            if (options_.quit_on_error)
                return false;

            if (!options_.ignore_errors) 
                continue;
        }

        if (options_.stop_after == Stage::layer1 ||
            options_.stop_after == Stage::layer2 ||
            options_.stop_after == Stage::layer3
        ) continue;

        if (compilation_state->empty())
            continue;

        unique_ptr<llvm::Module> module_ = make_unique<llvm::Module>(DEFAULT_MODULE_NAME, *context_);
        unique_ptr<IR::IR_Generator> generator = make_unique<IR::IR_Generator>(context_, module_.get(), 
            compilation_state.get(), error_stream_);

        insert_builtins(*generator);
        bool ir_success = generator->repl_run();
    
        if (options_.print_ir) {
            std::cerr << "---IR DUMP---:\n\n";
            module_->dump();
            std::cerr << "\n---IR END---\n";
        }
    
        if (!ir_success) {
            if (options_.quit_on_error)
                return false;
        
            if (!options_.ignore_errors)
                continue;
        }

        if (options_.stop_after == Stage::ir || !options_.eval)
            continue;

        std::cerr << prefix();
        eval(std::move(module_));
    }

    return true;
}

std::optional<std::string> REPL::get_input() {
    char* line = readline(options_.prompt ? PROMPT.cbegin() : "");

    if (!line) {
        std::cout << std::endl;
        running_ = false;
        return nullopt;
    }
    
    std::string input{line};
    
    if (input.empty()) {
        free(line);
        return nullopt;
    }
    
    add_history(line);
    free(line);
    save_history();

    return input;
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
    
void REPL::eval(std::unique_ptr<llvm::Module> module_) {
    jit_->reset();

    jit_->compile_and_run(static_cast<std::string>(IR::REPL_WRAPPER_NAME), std::move(module_));
    std::cout << std::endl;
}

std::string REPL::parse_type(std::istream& input_stream) {
    Maps::TypeStore types{};

    auto compilation_state = process_source(Maps::get_builtins(), &types, input_stream, parse_options_, std::cout);
    
    if (!compilation_state->is_valid && !options_.ignore_errors) {
        std::cout << "ERROR: parsing type failed" << std::endl;
        return "";
    }

    if (!compilation_state->entry_point_)
        return "Compilation didn't produce a value";

    auto type = (*compilation_state->entry_point_)->get_type();

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

    std::string next_arg;
    
    if (command == ":stop_after" || command == ":stop_at") {
        std::string next_arg;
        std::getline(input_stream, next_arg, ' ');

        if (next_arg == "layer1") {
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
        std::string next_arg;
        std::getline(input_stream, next_arg, ' ');

        if (next_arg == "eval") {
            options_.eval = !options_.eval;
            std::cout << "eval " << (options_.eval ? "on" : "off") << std::endl;
            return;
        }

        if (next_arg == "print" || next_arg == "show") { 
            std::string next_arg;
            std::getline(input_stream, next_arg, ' ');

            if (next_arg == "layer1") {
                options_.print_layer1 = !options_.print_layer1;
                update_parse_options();
                std::cout << "print layer1 " << (options_.eval ? "on" : "off") << std::endl;
                return;
            }

            if (next_arg == "layer2") {
                options_.print_layer2 = !options_.print_layer2;
                update_parse_options();
                std::cout << "print layer2 " << (options_.eval ? "on" : "off") << std::endl;
                return;
            }
            
            if (next_arg == "ir") {
                options_.print_ir = !options_.print_ir;
                std::cout << "print ir " << (options_.eval ? "on" : "off") << std::endl;
                return;
            }

            return;
        }

        if (next_arg == "stop_on_error") {
            options_.ignore_errors = !options_.ignore_errors;
            update_parse_options();
            std::cout << "stop_on_error " << (options_.eval ? "on" : "off") << std::endl;
            return;
        }

        std::cout << "\"" << next_arg << "\" is not a toggle" << std::endl;
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
    parse_options_.ignore_errors = options_.ignore_errors;
    parse_options_.print_layer1 = options_.print_layer1;
    parse_options_.print_layer2 = options_.print_layer2;
    parse_options_.print_layer3 = options_.print_layer3;

    parse_options_.ignore_errors = options_.ignore_errors;

    if (options_.stop_after == Stage::layer1) {
        parse_options_.stop_after = CompilationLayer::layer1;
        return;
    }

    if (options_.stop_after == Stage::layer2) {
        parse_options_.stop_after = CompilationLayer::layer2;
        return;
    }

    if (options_.stop_after == Stage::layer3) {
        parse_options_.stop_after = CompilationLayer::layer3;
        return;
    }

    parse_options_.stop_after = CompilationLayer::done;
}