#ifndef __JIT_MANAGER_HH
#define __JIT_MANAGER_HH

#include <string>

#include "llvm/ExecutionEngine/Orc/LLJIT.h"

namespace llvm {

class LLVMContext;
class Module;
class raw_ostream;

namespace orc { class ThreadSafeContext; }

} // namespace llvm


class JIT_Manager {
public:
    JIT_Manager(llvm::orc::ThreadSafeContext* context, llvm::raw_ostream* error_stream);

    // llvm::orc::ResourceTracker* compile_and_run_with_rt(llvm::Module& module) {
    //     auto resource_tracker = jit_->getMainJITDylib().createResourceTracker();
    // }

    bool compile_and_run(std::unique_ptr<llvm::Module> module, const std::string& entry_point);

    // resets everything in the jitDYlib
    void reset();

    bool is_good = true;
private:
    llvm::raw_ostream* error_stream_;
    llvm::orc::ThreadSafeContext* context_;
    std::unique_ptr<llvm::orc::LLJIT> jit_;
};


#endif