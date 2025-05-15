#include "init_llvm.hh"

#include "llvm/Support/TargetSelect.h"

// TODO: check errors
bool init_llvm() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    return true;
}