/**
 * This file contains the high level wrapper to run the parsing procedure
 */

#ifndef __FULL_PARSE_HH
#define __FULL_PARSE_HH

#include <tuple>
#include <memory>

#include "../lang/ast.hh"
#include "../lang/pragmas.hh"

std::tuple<std::unique_ptr<AST::AST>, std::unique_ptr<Pragma::Pragmas>>
    parse_source(std::istream& source_is);

#endif