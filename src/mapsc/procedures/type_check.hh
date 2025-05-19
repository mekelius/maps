#ifndef __TYPE_INFERENCE_HH
#define __TYPE_INFERENCE_HH

namespace Maps {

struct Expression;
struct Statement;
class RT_Callable;
class CompilationState;

class SimpleTypeChecker {
public:
    bool visit_expression(Expression*);
    bool visit_statement(Statement*);
    bool visit_callable(RT_Callable*);

    bool run(CompilationState& state);
};

} // namespace Maps

#endif