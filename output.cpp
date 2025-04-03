#include "output.hh"

using namespace llvm;

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

bool print_ir_to_file(const std::string& filename, Module* module_) {
    // prepare the stream
    std::error_code error_code; //??? what to do with this?
    raw_fd_ostream ostream{filename, error_code, sys::fs::OF_None};

    module_->print(ostream, nullptr);
    ostream.flush();

    return true;
}
