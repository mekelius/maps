#ifndef __OUTPUT_HH
#define __OUTPUT_HH

#include <string>
#include <ostream>

#include "llvm/IR/Module.h"

bool init_llvm_target();
bool generate_object_file(const std::string& filename, llvm::Module& module_, std::ostream& errs);

#endif