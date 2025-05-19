#ifndef __PARSE_DSIR_HH
#define __PARSE_DSIR_HH

#include <istream>
#include <optional>
#include <map>
#include <vector>

#include "mapsc/ast/scope.hh"

namespace Maps {

class CompilationState;
class Callable;
struct Expression;

namespace DSIR {

struct ParseResult {
    bool success = true;
    std::optional<Callable*> top_level_callable = std::nullopt;
    Scope definitions{};
    std::vector<Expression*> unresolved_identifiers{};
};

ParseResult eval_parse_dsir(CompilationState& state, std::istream& source);
ParseResult parse_dsir(CompilationState& state, std::istream& source);

} // namespace DSIR

} // namespace Maps

#endif