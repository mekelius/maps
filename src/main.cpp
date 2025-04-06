
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

using namespace llvm;

int main(int argc, char** argv) {
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
    
    std::unique_ptr<AST::AST> ast = parser.run(*cl_options);
    
    
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