/**
 * This file contains the high level wrapper to run the parsing procedure
 */

#ifndef __FULL_PARSE_HH
#define __FULL_PARSE_HH

#include <tuple>
#include <memory>
#include <optional>
#include <ostream>
#include <iostream>

#include "ast/ast.hh"
#include "lang/pragma.hh"

enum class ParseStage {
    layer1,
    layer2,
    done,    
};

struct ParseOptions {
    bool print_layer1 = false;
    bool print_layer2 = false;
    
    bool stop_on_error = true;
    ParseStage stop_after = ParseStage::done;
    
    bool in_repl;
};

// the first value is if the parse succeeded
std::tuple<bool, std::unique_ptr<Maps::AST>, std::unique_ptr<Pragma::Pragmas>>
    parse_source(std::istream& source_is, const ParseOptions& options = {}, 
        std::ostream& debug_print_ostream = std::cout);

#endif