
#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <iostream>
#include <fstream>
#include <sstream>

#include "llvm/IR/Module.h"

#include "config.hh"
#include "ir_generator.hh"
#include "output.hh"
#include "lexer.hh"
#include "parser.hh"

#if __cplusplus < 201703L
    #error(need c++17)
#endif

// TODO: handle multiple inputfiles
const std::string USAGE = "USAGE: testc inputfile [-o filename] [-ir filename] [-tokens filename] \n                        [-dump ir|tokens]";

const std::string DEFAULT_OBJ_FILE_PATH = "out.o";
const std::string DEFAULT_IR_FILE_PATH = "out.ll";
const std::string DEFAULT_TOKENS_FILE_PATH = "out.tokens";


enum class OutputSink {
    stderr,
    file,
    none
};

struct CL_Options {
    std::vector<std::string> input_file_paths = {};

    // TODO: implement cl-arg for wall
    bool wall = false; 

    OutputSink output_ir_to = OutputSink::none;
    OutputSink output_object_file_to = OutputSink::none; // NOTE: object file can't currectly be dumped to stderr
    OutputSink output_token_stream_to = OutputSink::none;
    
    std::string object_file_path = DEFAULT_OBJ_FILE_PATH;
    std::string ir_file_path = DEFAULT_IR_FILE_PATH;  
    std::string tokens_file_path = DEFAULT_TOKENS_FILE_PATH;
};


using namespace llvm;

std::optional<CL_Options> parse_cl_args(int argc, char** argv) {
    if (argc < 2)
        return std::nullopt;

    std::vector<std::string> args{ argv + 1, argv + argc };
    CL_Options options = {};

    for (auto it = args.begin(); it < args.end(); it++) {
        std::string arg = *it;

        if (arg == "-o" || arg == "-ir" || arg == "-tokens") {
            it++;

            if (it >= args.end())
                return std::nullopt;

            if (options.output_object_file_to != OutputSink::none) {
                std::cerr << "Conflicting command line arguments" << std::endl;
                return std::nullopt;
            }

            if (arg == "-o") {
                options.object_file_path = *it;
                options.output_object_file_to = OutputSink::file;
            } else if (arg == "-ir") {
                options.ir_file_path = *it;
                options.output_ir_to = OutputSink::file;
            } else if (arg == "-tokens") {
                options.tokens_file_path = *it;
                options.output_token_stream_to = OutputSink::file;
            }

            continue;            
        }
        
        if (arg == "-dump") {
            OutputSink sink = OutputSink::stderr;

            it++;
            if (it >= args.end())
                return std::nullopt;
            
            if (*it == "ir") {
                options.output_ir_to = sink;

            } else if (*it == "tokens") {
                options.output_token_stream_to = sink;

            } else {
                return std::nullopt;
            }

            continue;
        } 
        
        // args without '-' are input files
        if (options.input_file_paths.size() >= 1) {
            std::cerr << "Multiple input files not yet supported" << std::endl;
            return std::nullopt;
        }

        options.input_file_paths.push_back(arg);
    }

    return options;
}

int main(int argc, char** argv) {
    Logging::init();

    std::optional<CL_Options> cl_options = parse_cl_args(argc, argv);

    if (!cl_options) {
        std::cerr << USAGE << std::endl;
        return EXIT_FAILURE;
    }

    if (cl_options->input_file_paths.size() == 0) {
        std::cerr << "no input file" << std::endl;
        return EXIT_FAILURE;
    }

    std::string input_file_path = cl_options->input_file_paths.at(0);
    std::ifstream source_is{ input_file_path, std::ifstream::in};
    if (!source_is.is_open()) {
        std::cerr << "Couldn't open file: " << input_file_path << std::endl;
        return EXIT_FAILURE;
    }

    // prepare the output stream
    std::unique_ptr<std::ofstream> tokens_file;
    std::ostream* tokens_ostream;
    
    switch (cl_options->output_token_stream_to) {
        case OutputSink::file:
            tokens_file = std::make_unique<std::ofstream>(cl_options->tokens_file_path);
            tokens_ostream = tokens_file.get();
            break;
        case OutputSink::stderr:
            tokens_ostream = &std::cerr;
            break;
        case OutputSink::none:
            tokens_ostream = nullptr;
            break;
    }

    // ----- parse the source -----
    
    StreamingLexer lexer{&source_is, tokens_ostream};
    Parser parser{&lexer, &std::cerr/*, &ir_gen_helper*/};
    
    // if tokens get dumped, provide clearer separation
    if (cl_options->output_token_stream_to == OutputSink::stderr) {
        std::cerr << "\n" << "--- START PARSING ---" << "\n\n";
    } else {
        std::cerr << "Parsing source file(s)...\n";
    }
    
    std::unique_ptr<AST::AST> ast = parser.run();

    // if tokens get dumped, provide clearer separation
    if (cl_options->output_token_stream_to == OutputSink::stderr) {
        std::cerr << "\n" << "--- PARSING COMPLETE ---" << "\n" << std::endl;
    } else {
        std::cerr << "Parsing complete" << std::endl;
    }
    
    
    // ----- CODE GEN -----
    std::cerr << "Initializing llvm module and target" << std::endl;

    // ----- initialize llvm -----
    if (!init_llvm()) {
        std::cerr << "Couldn't initialize llvm, exiting" << std::endl;
        return EXIT_FAILURE;
    }

    std::string module_name = "Test module";
    IR_Generator ir_gen_helper{module_name, &std::cerr};
    
    // ----- run codegen -----
    std::cerr << "Running codegen..." << std::endl;

    if (!ir_gen_helper.generate_ir(ast.get())) {
        std::cerr << "Codegen failed, exiting" << std::endl;
        return EXIT_FAILURE;
    }

    std::cerr << "Codegen finished" << std::endl;
    
    // ----- produce output -----
    
    Module* module_ = ir_gen_helper.get_module();
    if (cl_options->output_ir_to == OutputSink::file) {
        std::cerr << "outputting ir to " << cl_options->ir_file_path << std::endl;
        print_ir_to_file(cl_options->ir_file_path, module_);
    }

    if (cl_options->output_ir_to == OutputSink::stderr) {
        std::cerr << "--- GENERATED IR ----\n" << std::endl;
        module_->dump();
        std::cerr << "\n------ IR ENDS ------\n" << std::endl;
    }

    if (cl_options->output_object_file_to == OutputSink::file) {
        std::cerr << "outputting object file to " << cl_options->object_file_path << std::endl;
        generate_object_file(cl_options->object_file_path, module_);
    }
    
    std::cerr << "done" << std::endl;
    return EXIT_SUCCESS;
}