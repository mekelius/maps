#include "jit_manager.hh"

#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/ExecutionEngine/Orc/Shared/ExecutorAddress.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"


JIT_Manager::JIT_Manager(llvm::orc::ThreadSafeContext* context, llvm::raw_ostream* error_stream)
    : error_stream_(error_stream), context_(context) {

    auto jit = llvm::orc::LLJITBuilder().create();

    if (!jit) {
        *error_stream_ << jit.takeError() << '\n';
        is_good = false;
        return;
    }

    jit_ = std::move(*jit);
}

// llvm::orc::ResourceTracker* compile_and_run_with_rt(llvm::Module& module) {
//     auto resource_tracker = jit_->getMainJITDylib().createResourceTracker();
// }
    
bool JIT_Manager::compile_and_run(const std::string& name, std::unique_ptr<llvm::Module> module) {
    if (auto err = jit_->addIRModule(llvm::orc::ThreadSafeModule{std::move(module), *context_})) {
        *error_stream_ << err << '\n';
        error_stream_->flush();
        return false;
    }

    auto f1_sym = jit_->lookup(name);
    if (!f1_sym) {
        *error_stream_ << f1_sym.takeError() << '\n';
        error_stream_->flush();
        return false;
    }

    // execute the compiled function
    auto *f1_ptr = f1_sym->toPtr<void(*)()>();
    f1_ptr();

    return true;
}

// resets everything in the jitDYlib
void JIT_Manager::reset() {
    auto error = jit_->getMainJITDylib().clear();
    if (error) {
        *error_stream_ << error;
        error_stream_->flush();
    }
}
    