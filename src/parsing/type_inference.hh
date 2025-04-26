#ifndef __TYPE_INFERENCE_HH
#define __TYPE_INFERENCE_HH

#include "../lang/ast.hh"

namespace Maps {

class SimpleTypeChecker {
public:
    void visit_expression(Expression*);
    void visit_statement(Statement*);
    void visit_callable(Callable*);

    bool run(AST& ast);

private:
    void fail() { success = false; };

    bool success = true;
};

} // namespace Maps

#endif