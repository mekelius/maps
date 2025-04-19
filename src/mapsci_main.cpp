#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "logging.hh"
#include "lang/pragmas.hh"
#include "parsing/full_parse.hh"
#include "ir/ir_generator.hh"

using Logging::LogLevel;

constexpr std::string_view USAGE = "USAGE: testci [source_file...]";
constexpr std::string_view DEFAULT_MODULE_NAME = "interpreted";
constexpr std::string_view PROMPT = "mapsci> ";

class REPL {
public:
    struct Options {
        bool print_reverse_parse = false;
        bool print_ir = false;
        bool eval = true;
    };

    REPL() {
        ir_generator_ = std::make_unique<IR::IR_Generator>(
            static_cast<std::string>(DEFAULT_MODULE_NAME), &std::cerr);
    }

    REPL(Options options): options_(options) {
        REPL();
    }

    void run() {    
        while (running) {
            std::cout << PROMPT;
    
            std::string input;
            std::getline(std::cin, input);
    
            if (std::cin.eof()) {
                std::cout << std::endl;
                running = false;
            }
    
            if (input.empty())
                continue;
        
            if (input.at(0) == ':') {
                run_command(input);
                continue;
            }
            
            std::stringstream input_s{input};
            std::tie(ast_, pragmas_) = parse_source(input_s);

            ir_generator_->run(*ast_, pragmas_.get());
            ir_generator_->module_->dump();
        }
    }

    void run_command(const std::string& command) {
        if (command == ":q"     || 
            command == ":quit"  || 
            command == ":e"     || 
            command == ":exit"  || 
            command == ":c"     ||
            command == ":close"     
        ) running = false;
    }

private:
    bool running = true;

    Options options_ = {};

    std::unique_ptr<IR::IR_Generator> ir_generator_;
    std::unique_ptr<AST::AST> ast_{};
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
    // for(std::string filename: filenames) {

    // }

    // if 
    REPL{}.run();

    return EXIT_SUCCESS;
}