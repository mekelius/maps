#include "implementation.hh"

#include <iostream>

namespace Maps {

void REPL::run_command(Maps::CompilationState& state, const std::string& input) {
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
    
    // if (command == ":names") {
    //     if (state.globals_.empty())
    //         std::cout << "Global scope is empty\n";

    //     for (auto [name, definition]: state.globals_.identifiers_in_order_) {
    //         std::cout << name << "\n";
    //     }
        
    //     return;
    // }

    if (command == ":stop_after" || command == ":stop_at") {
        std::string next_arg;
        std::getline(input_stream, next_arg, ' ');

        if (next_arg == "layer1") {
            options_.stop_after = REPL_Stage::layer1;
            return;
        }

        if (next_arg == "layer2") {
            options_.stop_after = REPL_Stage::layer2;
            return;
        } 
        
        if (next_arg == "ir") {
            options_.stop_after = REPL_Stage::ir;
            return;
        }
        
        if (next_arg == "done" || next_arg == "eval") {
            options_.stop_after = REPL_Stage::done;
            return;
        }

        std::cout << next_arg << " is not a valid point to stop\n"; 
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

        if (next_arg == "stop_on_error") {
            options_.ignore_errors = !options_.ignore_errors;
            std::cout << "stop_on_error " << (options_.eval ? "on" : "off") << std::endl;
            return;
        }

        std::cout << "\"" << next_arg << "\" is not a toggle" << std::endl;
        return;
    }

    if (command == ":t") {
        std::cout << eval_type(input_stream) << std::endl;
        return;
    }
    
    std::cout << "\"" << command << "\" is not a command" << std::endl;
    return;
}

} // namespace Maps
