#ifndef __TYPE_INFERENCE_HH
#define __TYPE_INFERENCE_HH

#include "mapsc/ast/scope.hh"

namespace Maps {

struct Expression;
struct Statement;
class DefinitionBody;
class CompilationState;

bool type_check(DefinitionBody& definition);

class SimpleTypeChecker {
public:
    bool visit_expression(Expression*);
    bool visit_statement(Statement*);
    bool visit_definition(DefinitionBody*);

    bool run(CompilationState& state, Scope scope, 
        std::span<DefinitionBody* const> extra_definitions);
};

} // namespace Maps

#endif