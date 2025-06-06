#include "implementation.hh"
#include "../repl.hh"

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

#include "mapsc/procedures/reverse_parse.hh"
#include "mapsc/types/type_store.hh"

using std::optional, std::nullopt;
using std::unique_ptr, std::make_unique, std::make_optional, std::tuple;

namespace Maps {

// ----- PUBLIC METHODS -----

REPL::REPL(JIT_Manager* jit, llvm::LLVMContext* context, llvm::raw_ostream* error_stream, 
    const REPL_Options& options)
:context_(context), 
 jit_(jit), 
 error_stream_(error_stream), 
 options_(options), 
 reverse_parser_(ReverseParser{&std::cout, options.reverse_parse}) {
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
:REPL(jit, context, error_stream, {}) {}

bool REPL::run() {    
    auto types = TypeStore{};
    auto stored_state = CompilationState{&types, options_.compiler_options};
    auto stored_definitions = Maps::Scope{};

    while (running_) {        
        optional<std::string> input = get_input();

        if (!input)
            continue;

        if (input->at(0) == ':') {
            run_command(stored_state, *input);
            continue;
        }

        std::stringstream source{*input};

        auto state = stored_state;
        auto definitions = stored_definitions;

        if (!run_compilation_pipeline(state, definitions, source)) {
            if (options_.quit_on_error)
                return false;
            continue;
        }

        stored_state = state;
        stored_definitions = definitions;
    }

    return true;
}

void REPL::debug_print(REPL_Stage stage, const Maps::Scope& scope, 
    std::span<const Maps::DefinitionHeader* const> extra_definitions) {

    if (!options_.has_debug_print(stage))
        return;

    auto [start_separator, end_separator] = debug_print_separators.at(static_cast<size_t>(stage));
    reverse_parser_ 
        << '\n' << std::string{start_separator} << '\n' 
        << scope;

        for (auto definition: extra_definitions) {
            reverse_parser_ << *definition;
        }

        reverse_parser_ << '\n' << std::string{end_separator} << "\n\n";
}

void REPL::debug_print(REPL_Stage stage, const Maps::Scope& scope, const Maps::DefinitionHeader* definition) {
    debug_print(stage, scope, std::array{definition});
}

void REPL::debug_print(REPL_Stage stage, const Maps::Scope& scope) {
    debug_print(stage, scope, {});
}

void REPL::debug_print(REPL_Stage stage, const llvm::Module& module) {
    if (!options_.has_debug_print(stage))
        return;

    auto [start_separator, end_separator] = debug_print_separators.at(static_cast<size_t>(stage));
    
    reverse_parser_ << '\n' << std::string{start_separator} << "\n\n";
    module.dump();
    reverse_parser_ << '\n' << std::string{end_separator} << "\n\n";
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

} // namespace Maps
