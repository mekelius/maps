#include "type_check.hh"

#include <cassert>
#include <optional>
#include <string>
#include <variant>

#include "common/std_visit_helper.hh"

#include "mapsc/source.hh"
#include "mapsc/logging.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/types/type.hh"

#include "mapsc/ast/callable.hh"
#include "mapsc/ast/expression.hh"

#include "mapsc/procedures/concretize.hh"
#include "mapsc/procedures/coerce_type.hh"

using std::get, std::get_if, std::optional, std::nullopt;

namespace Maps {

using Log = LogInContext<LogContext::layer4>;

bool SimpleTypeChecker::visit_expression(Expression* expression) {
    if (expression->declared_type && **expression->declared_type != *expression->type)
        return handle_declared_type(*expression, *expression->declared_type);

    if (concretize(*expression))
        return true;

    Log::error("Found a non-reduced type: " + static_cast<std::string>(expression->type->name()), 
        expression->location);

    return false;
}

bool SimpleTypeChecker::visit_callable(RT_Callable* callable) {
    return std::visit(overloaded{
        [](External) { return true; },
        [](Undefined) { return true; },
        [](Expression*) { return true; },
        [callable](Statement* statement) {                        
            auto type = callable->get_type();
            auto declared_type = callable->get_declared_type();

            if (declared_type) {
                if (type && *type != **declared_type) {
                    assert(false && 
                        "in visit_callable: declared type mismatch, handling not implemented");
                    return false;
                }
            }

            // if (*type == Hole)
                // get return type and set it

            return coerce_return_type(*statement, type);
        }
    }, callable->body());
}

// Note, statements shouldn't mess with contained expressions, since they will be visited
bool SimpleTypeChecker::visit_statement(Statement* statement) { 
    return true; 
}

bool SimpleTypeChecker::run(CompilationState& state) {
    // return ast.walk_tree(*this);
    // !!! ignoring failures
    state.walk_tree(*this);
    return true;
}

} // namespace Maps
