#ifndef __PARSER_LAYER_1_HH
#define __PARSER_LAYER_1_HH

#include <istream>
#include <optional>
#include <vector>

#include "mapsc/ast/scope.hh"

namespace Maps {

class CompilationState;
struct Expression;

struct Layer1Result {
    bool success = true;
    std::optional<RT_Definition*> top_level_definition;
    std::vector<Expression*> unresolved_identifiers;
    std::vector<Expression*> unresolved_type_identifiers;
    std::vector<Expression*> unparsed_termed_expressions;
    std::vector<Expression*> possible_binding_type_declarations;
};

Layer1Result run_layer1(CompilationState& state, RT_Scope& scope, std::istream& source_is);
Layer1Result run_layer1_eval(CompilationState& state, RT_Scope& scope, std::istream& source_is);

} // namespace Maps

#endif