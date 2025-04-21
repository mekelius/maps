#ifndef __JIT_MANAGER_HH
#define __JIT_MANAGER_HH

#include <functional>

#include "llvm/IR/Module.h"

#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"

class JIT_Manager {
public:
    JIT_Manager(llvm::orc::ThreadSafeContext);

    std::function<void> clear_and_insert_module(llvm::Module& module);

private:
};

#endif