#include "layer2.hh"
#include "layer2/implementation.hh"

#include "mapsc/logging.hh"
#include "mapsc/ast/expression.hh"

namespace Maps {

using Log = LogInContext<LogContext::layer2>;

bool run_layer2(CompilationState& state, Expression* expression) {    
    Log::debug_extra(expression->location) << "Running layer2 on " << *expression;
    return TermedExpressionParser{&state, expression}.run();
}

bool run_layer2(CompilationState& state, std::vector<Expression*>& unparsed_termed_expressions) {
    for (Expression* expression: unparsed_termed_expressions) {
        if (!run_layer2(state, expression))
            return false;
    }
    return true;
}

} // namespace Maps
