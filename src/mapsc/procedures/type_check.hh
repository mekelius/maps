#ifndef __TYPE_INFERENCE_HH
#define __TYPE_INFERENCE_HH

#include "mapsc/ast/scope.hh"

namespace Maps {

struct Expression;
struct Statement;
class RT_Definition;
class CompilationState;

class SimpleTypeChecker {
public:
    bool visit_expression(Expression*);
    bool visit_statement(Statement*);
    bool visit_definition(RT_Definition*);

    bool run(CompilationState& state, Scopes scopes, 
        std::span<RT_Definition* const> extra_definitions);
};

} // namespace Maps

#endif