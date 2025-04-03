#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "llvm/Support/Host.h"
#include "llvm/Support/CodeGen.h"

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

#include "ir_gen.hh"
#include "parsing.hh"

using namespace llvm;

// ----- Constants -----

const std::string IR_FILE_NAME = "./build/out.ll";

// -------------------------

bool init_llvm() {
    // if (InitializeNativeTarget() != 0)
    //     return false;
    
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

bool generate_object_file(const std::string& filename, Module* module_) {
    // check if we have a target
    auto target_triple = sys::getDefaultTargetTriple();
    std::string error;
    auto target = TargetRegistry::lookupTarget(target_triple, error);
    
    if (!target) {
        std::cout << "No target: " << error << std::endl;
        return false;
    }
    
    auto cpu = "generic";
    auto features = "";

    TargetOptions opt;
    auto target_machine = target->createTargetMachine(target_triple, cpu, features, opt, Reloc::PIC_);

    // prepare the stream
    std::error_code error_code;
    raw_fd_ostream output(filename, error_code, sys::fs::OF_None);

    legacy::PassManager pass;

    if (target_machine->addPassesToEmitFile(pass, output, nullptr, CGFT_ObjectFile)) {
        std::cerr << "Couldn't generate object file for target machine" << std::endl;
        return false;
    }

    module_->setDataLayout(target_machine->createDataLayout());
    module_->setTargetTriple(target_triple);

    // output the file
    pass.run(*module_);
    output.flush();

    return true;
}

bool print_ir(const std::string& filename, Module* module_) {
    // prepare the stream
    std::error_code error_code;
    raw_fd_ostream output(filename, error_code, sys::fs::OF_None);

    module_->print(output, nullptr);
    output.flush();

    return true;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "USAGE: testc source_file object_file" << std::endl;
        return EXIT_FAILURE; 
    }

    std::string source_filename = argv[1];
    std::string object_file_name = argv[2];

    // ----- read the source into a buffer -----
    std::string source = "";
    if (!read_source_file(source_filename, source))
        return EXIT_FAILURE;
    std::istringstream iss{source};
    std::istream source_is{iss.rdbuf()};

    // ----- initialize llvm -----
    if (!init_llvm()) {
        std::cerr << "Couldn't initialize llvm, exiting" << std::endl;
        return EXIT_FAILURE;
    }

    // ----- parse the source -----
    std::string module_name = "Test module";

    
    StreamingLexer lexer{&source_is};
    IRGenHelper ir_gen_helper{module_name};
    DirectParser parser{&lexer, &ir_gen_helper};
    
    parser.run();
    
    // ----- produce output -----
    Module* module_ = ir_gen_helper.get_module();
    print_ir(IR_FILE_NAME, module_);
    generate_object_file(object_file_name, module_);

    return EXIT_SUCCESS;
}