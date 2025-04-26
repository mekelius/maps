/**
 * This file contains the high level wrapper to run the parsing procedure
 */

#ifndef __FULL_PARSE_HH
#define __FULL_PARSE_HH

#include <tuple>
#include <memory>
#include <optional>

#include "../lang/ast.hh"
#include "../lang/pragma.hh"

std::optional<std::tuple<std::unique_ptr<Maps::AST>, std::unique_ptr<Pragma::Pragmas>>>
    parse_source(std::istream& source_is, bool in_repl = false, bool ignore_errors = false);

#endif