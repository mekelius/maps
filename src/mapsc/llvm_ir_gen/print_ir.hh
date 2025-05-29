#ifndef __PRINT_IR_HH
#define __PRINT_IR_HH

#include <string>
#include <stdexcept>
#include "llvm/IR/Module.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

inline bool print_ir_to_file(const std::string& filename, const llvm::Module& module) {
    // prepare the stream
    std::error_code error_code; //??? what to do with this?
    llvm::raw_fd_ostream ostream{filename, error_code, llvm::sys::fs::OF_None};

    module.print(ostream, nullptr);
    ostream.flush();

    return true;
}

#endif