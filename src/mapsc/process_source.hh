#ifndef __PROCESS_SOURCE_HH
#define __PROCESS_SOURCE_HH

/**
 * This file contains the high level wrapper to run the whole parsing procedure
 */

#include <tuple>
#include <memory>
#include <optional>
#include <ostream>
#include <iostream>

#include "mapsc/pragma.hh" // NOTE: these need to be included since the user of unique_ptr needs to know how to destroy it
#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/builtin.hh"

namespace Maps {

enum class CompilationLayer {
    layer1,
    layer2,
    post_layer_2,
    done,
};

struct ProcessSourceOptions {
    bool print_layer1 = false;
    bool print_layer2 = false;
    
    bool ignore_errors = false;
    CompilationLayer stop_after = CompilationLayer::done;
    
    bool in_repl;
};

// the first value is if the parse succeeded
std::tuple<bool, std::unique_ptr<AST_Store>, std::unique_ptr<PragmaStore>>
    process_source(std::istream& source_is, const ProcessSourceOptions& options = {}, 
        std::ostream& debug_ostream = std::cout);

} // namespace Maps

#endif