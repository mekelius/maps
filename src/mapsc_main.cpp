
#include <algorithm>
#include <cassert>
#include <map>
#include <tuple>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <iostream>
#include <fstream>
#include <sstream>

#include "llvm/IR/Module.h"

#include "logging.hh"

#include "lang/ast.hh"

#include "parsing/full_parse.hh"

#include "ir/ir_generator.hh"
#include "ir/ir_output.hh"

// TODO: handle multiple inputfiles
constexpr std::string_view USAGE = "USAGE: testc inputfile [-o filename] [-ir filename]";

constexpr std::string_view DEFAULT_MODULE_NAME = "module";

constexpr std::string_view DEFAULT_OBJ_FILE_PATH = "out.o";
constexpr std::string_view DEFAULT_IR_FILE_PATH = "out.ll";

struct CL_Options {
    std::vector<std::string> input_file_paths = {};

    // TODO: implement cl-arg for wall
    bool wall = false; 

    bool object_file = true;
    std::string object_file_path = static_cast<std::string>(DEFAULT_OBJ_FILE_PATH);

    bool ir_file = false;
    bool print_ir = false;
    std::string ir_file_path = static_cast<std::string>(DEFAULT_IR_FILE_PATH);
};

std::optional<CL_Options> parse_cl_args(int argc, char** argv) {
    if (argc < 2)
        return std::nullopt;

    std::vector<std::string> args{ argv + 1, argv + argc };
    CL_Options options = {};

    for (auto it = args.begin(); it < args.end(); it++) {
        std::string arg = *it;

        if (arg == "-o" || arg == "-ir") {
            it++;

            if (it >= args.end())
                return std::nullopt;

            if (arg == "-o") {
                options.object_file_path = *it;
            } else if (arg == "-ir") {
                options.ir_file_path = *it;
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
    Logging::init_logging(&std::cerr);

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
    
    // ----- parse the source -----
    
    std::cerr << "Parsing source file(s)...\n";

    std::unique_ptr<AST::AST> ast;
    std::unique_ptr<Pragma::Pragmas> pragmas;

    std::tie(ast, pragmas) = parse_source(source_is);

    std::cerr << "Parsing complete" << std::endl;
    
    // ----- CODE GEN -----
    std::cerr << "Initializing llvm module and target" << std::endl;

    // ----- initialize llvm -----
    if (!init_llvm()) {
        std::cerr << "Couldn't initialize llvm, exiting" << std::endl;
        return EXIT_FAILURE;
    }

    IR::IR_Generator ir_gen_helper{static_cast<std::string>(DEFAULT_MODULE_NAME), &std::cerr};
    
    // ----- run codegen -----
    std::cerr << "Running codegen..." << std::endl;

    if (!ir_gen_helper.run(*ast)) {
        std::cerr << "Codegen failed, exiting" << std::endl;
        return EXIT_FAILURE;
    }

    std::cerr << "Codegen finished" << std::endl;
    
    // ----- produce output -----
    
    llvm::Module* module_ = ir_gen_helper.module_.get();
    if (cl_options->ir_file) {
        std::cerr << "outputting ir to " << cl_options->ir_file_path << std::endl;
        print_ir_to_file(cl_options->ir_file_path, module_);
    }

    if (cl_options->print_ir) {
        std::cerr << "--- GENERATED IR ----\n" << std::endl;
        module_->dump();
        std::cerr << "\n------ IR ENDS ------\n" << std::endl;
    }

    if (cl_options->object_file) {
        std::cerr << "outputting object file to " << cl_options->object_file_path << std::endl;
        generate_object_file(cl_options->object_file_path, module_);
    }
    
    std::cerr << "done" << std::endl;
    return EXIT_SUCCESS;
}