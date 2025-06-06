#ifndef __CREATE_CALL_HH
#define __CREATE_CALL_HH

#include <span>
#include <utility>
#include <vector>

#include "mapsc/source_location.hh"

namespace Maps {

class AST_Store;
class CompilationState;
struct Expression;
class Type;
class DefinitionHeader;

// return values are: <bool success, bool partial, bool is done, const Type* return_type>
std::tuple<bool, bool, bool, const Type*> check_and_coerce_args(CompilationState& state, 
    const DefinitionHeader* callee, std::vector<Expression*>& args, const SourceLocation& location);

} // namespace Maps

#endif