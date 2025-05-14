#include "obj_output.hh"

#include <optional>
#include <system_error>

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/CodeGen.h"


using namespace llvm;

bool init_llvm_target() {
    // TODO: how to detect failure

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    // InitializeNativeTarget();
    // InitializeNativeTargetAsmPrinter();
    // InitializeNativeTargetAsmParser();

    return true;
}

bool generate_object_file(const std::string& filename, Module& module_, std::ostream& errs) {
    // check if we have a target
    auto target_triple = sys::getDefaultTargetTriple();
    std::string error;
    auto target = TargetRegistry::lookupTarget(target_triple, error);
    
    if (!target) {
        errs << "No target: " << error << std::endl;
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

    if (target_machine->addPassesToEmitFile(pass, output, nullptr, CodeGenFileType::ObjectFile)) {
        errs << "Couldn't generate object file for target machine" << std::endl;
        return false;
    }

    module_.setDataLayout(target_machine->createDataLayout());
    module_.setTargetTriple(target_triple);

    // output the file
    pass.run(module_);
    output.flush();

    return true;
}
