#ifndef __TYPE_INFERENCE_HH
#define __TYPE_INFERENCE_HH

#include "../lang/ast.hh"

namespace Maps {

class SimpleTypeChecker {
public:
    bool visit_expression(Expression*);
    bool visit_statement(Statement*);
    bool visit_callable(Callable*);

    bool run(AST& ast);
};

} // namespace Maps

#endif