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

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/ast_node.hh"

#include "mapsc/procedures/concretize.hh"
#include "mapsc/procedures/coerce_type.hh"

using std::get, std::get_if, std::optional, std::nullopt;

namespace Maps {

using Log = LogInContext<LogContext::layer4>;


bool type_check(Definition& definition) {
    assert(false && "not implemented");
}


bool SimpleTypeChecker::visit_expression(Expression* expression) {
    if (expression->declared_type && **expression->declared_type != *expression->type)
        return handle_declared_type(*expression, *expression->declared_type);

    if (concretize(*expression))
        return true;

    Log::error("Concretizing " + expression->log_message_string() + " failed", 
        expression->location);

    return false;
}

bool SimpleTypeChecker::visit_definition(RT_Definition* definition) {
    return std::visit(overloaded{
        [](Error) { return false; },
        [](External) { return true; },
        [](Undefined) { return true; },
        [](Expression*) { return true; },
        [](BTD_Binding) { return true; },
        [definition](Statement* statement) {                        
            auto type = definition->get_type();
            auto declared_type = definition->get_declared_type();

            if (declared_type) {
                if (type && *type != **declared_type) {
                    assert(false && 
                        "in visit_definition: declared type mismatch, handling not implemented");
                    return false;
                }
            }

            // if (*type == Hole)
                // get return type and set it

            return coerce_return_type(*statement, type);
        }
    }, definition->body());
}

// Note, statements shouldn't mess with contained expressions, since they will be visited
bool SimpleTypeChecker::visit_statement(Statement* statement) { 
    return true; 
}

bool SimpleTypeChecker::run(CompilationState& state, Scopes scopes, 
    std::span<RT_Definition* const> extra_definitions) {

    for (auto scope: scopes) {
        for (auto [_1, definition]: scope->identifiers_in_order_) {
            if (!walk_definition(*this, definition))
                return false;
        }
    }

    for (auto definition: extra_definitions) {
        if (!walk_definition(*this, definition))
            return false;
    }

    return true;
}

} // namespace Maps
