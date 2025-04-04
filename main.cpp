
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
#include "ir_gen.hh"
#include "output.hh"
#include "lexer.hh"
#include "parser.hh"

using namespace llvm;

bool init_llvm() {
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();    

    return true;
}

bool read_source_file(const std::string& filename, std::string& source) {
    std::ifstream source_file{filename, std::ifstream::in};
    if (!source_file.is_open()) {
        std::cerr << "Couldn't open file: " << filename << std::endl;
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(source_file)), std::istreambuf_iterator<char>());
    source = content;

    source_file.close();
    return true;
}

int main(int argc, char** argv) {
    CL_Options cl_options = parse_cl_args(argc, argv);

    if (cl_options.bad_args) {
        std::cout << USAGE << std::endl;
        return EXIT_FAILURE;
    }

    if (cl_options.input_file_paths.size() == 0) {
        std::cout << "no input file" << std::endl;
        return EXIT_FAILURE;
    }

    // ----- read the source into a buffer -----
    // TODO: replace with a proper buffer
    std::string source = "";
    if (!read_source_file(cl_options.input_file_paths.front(), source))
        return EXIT_FAILURE;
    std::istringstream iss{source};
    std::istream source_is{iss.rdbuf()};

    // ----- initialize llvm -----
    if (!init_llvm()) {
        std::cerr << "Couldn't initialize llvm, exiting" << std::endl;
        return EXIT_FAILURE;
    }

    // prepare the output streams
    std::unique_ptr<std::ofstream> tokens_file;
    std::ostream* tokens_ostream;
    
    switch (cl_options.output_token_stream_to) {
        case OutputSink::file:
            tokens_file = std::make_unique<std::ofstream>(cl_options.tokens_file_path);
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
    std::string module_name = "Test module";

    StreamingLexer lexer{&source_is, tokens_ostream};
    IRGenHelper ir_gen_helper{module_name};
    Parser parser{&lexer, &std::cerr/*, &ir_gen_helper*/};
    
    parser.run();
    
    // ----- produce output -----
    Module* module_ = ir_gen_helper.get_module();

    if (cl_options.output_ir_to == OutputSink::file)
        print_ir_to_file(cl_options.ir_file_path, module_);

    if (cl_options.output_ir_to == OutputSink::stderr) {
        std::cerr << "--- GENERATED IR ----\n" << std::endl;
        module_->dump();
        std::cerr << "\n------ IR ENDS ------\n" << std::endl;
    }

    if (cl_options.output_object_file_to == OutputSink::file)
        generate_object_file(cl_options.object_file_path, module_);
    

    return EXIT_SUCCESS;
}