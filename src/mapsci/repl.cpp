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
#include "mapsc/ast/definition.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/process_source.hh"
#include "mapsc/procedures/reverse_parse.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/types/type.hh"

#include "mapsc/llvm/ir_generator.hh"


using std::optional, std::nullopt;
using std::unique_ptr, std::make_unique, std::make_optional, std::tuple;
using Maps::ProcessSourceOptions, Maps::ReverseParser, Maps::CompilationLayer;
using Maps::TypeStore, Maps::CompilationState;

// ----- PUBLIC METHODS -----

REPL::REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream, 
    Options options)
: context_(context), jit_(jit), error_stream_(error_stream), options_(options) {
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
    auto types = TypeStore{};
    auto stored_state = CompilationState{Maps::get_builtins(), &types, options_.compiler_options};
    auto stored_scope = Maps::RT_Scope{};

    while (running_) {        
        optional<std::string> input = get_input();

        if (!input)
            continue;

        if (input->at(0) == ':') {
            run_command(stored_state, *input);
            continue;
        }

        auto compilation_state = stored_state;
        auto compilation_scope = stored_scope;
        std::stringstream input_s{*input};

        if (!run_compilation_pipeline(compilation_state, compilation_scope, input_s)) {
            if (options_.quit_on_error)
                return false;
            continue;
        }

        stored_state = compilation_state;
        stored_scope = compilation_scope;
    }

    return true;
}

// ----- PRIVATE METHODS -----

std::optional<std::string> REPL::get_input() {
    char* line = readline(options_.prompt.c_str());

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
