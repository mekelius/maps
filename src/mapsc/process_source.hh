#ifndef __PROCESS_SOURCE_HH
#define __PROCESS_SOURCE_HH

/**
 * This file contains the high level wrapper to run the whole parsing procedure
 */

#include <memory>
#include <iostream>

#include "mapsc/compilation_state.hh"

namespace Maps {

class Scope;
class TypeStore;

enum class CompilationLayer {
    layer1,
    layer2,
    layer3,
    done,
};

struct ProcessSourceOptions {
    bool print_layer1 = false;
    bool print_layer2 = false;
    bool print_layer3 = false;
    
    bool ignore_errors = false;
    CompilationLayer stop_after = CompilationLayer::done;
    
    bool in_repl;
};

// the first value is if the parse succeeded
std::unique_ptr<CompilationState> process_source(const Scope* builtins, TypeStore* types, std::istream& source_is, 
    const ProcessSourceOptions& options = {}, std::ostream& debug_ostream = std::cout);
std::unique_ptr<CompilationState> process_source(const CompilationState& base, std::istream& source_is, 
    const ProcessSourceOptions& options = {}, std::ostream& debug_ostream = std::cout);

} // namespace Maps

#endif