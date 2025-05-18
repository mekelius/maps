#ifndef __PARSE_DSIR_HH
#define __PARSE_DSIR_HH

#include <istream>
#include <optional>

namespace Maps {

class CompilationState;
class Scope;
class Callable;

std::optional<Callable*> eval_parse_dsir(CompilationState& state, std::istream& source);
bool parse_dsir(CompilationState& state, std::istream& source);

} // namespace Maps

#endif