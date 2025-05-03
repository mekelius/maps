#ifndef __TYPE_INFERENCE_HH
#define __TYPE_INFERENCE_HH

#include "mapsc/ast/ast_store.hh"

namespace Maps {

class SimpleTypeChecker {
public:
    bool visit_expression(Expression*);
    bool visit_statement(Statement*);
    bool visit_callable(Callable*);

    bool run(AST_Store& ast);
};

} // namespace Maps

#endif