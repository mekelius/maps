#ifndef __FULL_PARSE_HH
#define __FULL_PARSE_HH

/**
 * This file contains the high level wrapper to run the whole parsing procedure
 */

#include <tuple>
#include <memory>
#include <optional>
#include <ostream>
#include <iostream>

#include "mapsc/ast/ast_store.hh"
#include "mapsc/pragma.hh"

enum class ParseStage {
    layer1,
    layer2,
    done,    
};

struct ParseOptions {
    bool print_layer1 = false;
    bool print_layer2 = false;
    
    bool ignore_errors = false;
    ParseStage stop_after = ParseStage::done;
    
    bool in_repl;
};

// the first value is if the parse succeeded
std::tuple<bool, std::unique_ptr<Maps::AST_Store>, std::unique_ptr<Maps::PragmaStore>>
    parse_source(std::istream& source_is, const ParseOptions& options = {}, 
        std::ostream& debug_ostream = std::cout);

#endif