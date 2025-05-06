#ifndef __TYPE_INFERENCE_HH
#define __TYPE_INFERENCE_HH

#include "mapsc/ast/ast_store.hh"

namespace Maps {

class TypeConcretizer {
public:
    static bool concretize_expression(Expression& expression);
    static bool concretize_call(Expression& call);
    static bool concretize_value(Expression& value);
};

class SimpleTypeChecker {
public:
    bool visit_expression(Expression*);
    bool visit_statement(Statement*);
    bool visit_callable(Callable*);

    bool run(AST_Store& ast);
};

} // namespace Maps

#endif