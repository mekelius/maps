#include "type_check.hh"

#include <cassert>
#include <optional>
#include <string>
#include <variant>

#include "common/std_visit_helper.hh"

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/types/type.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/definition_body.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/ast_node.hh"

#include "mapsc/procedures/coerce_type.hh"

using std::get, std::get_if, std::optional, std::nullopt;

namespace Maps {

using Log = LogInContext<LogContext::layer4>;


bool type_check(DefinitionBody& definition) {
    return std::visit(overloaded{
        [](Expression* expression) { return SimpleTypeChecker{}.visit_expression(expression); },
        [](auto) { return true; }
    }, definition.body());
}


bool SimpleTypeChecker::visit_expression(Expression* expression) {
    if (expression->declared_type && **expression->declared_type != *expression->type)
        return handle_declared_type(*expression, *expression->declared_type);

    switch (expression->expression_type) {
        case ExpressionType::known_value:
            return true;

        case ExpressionType::call:
            
        default:
            assert(false && "Unexpected expressiontype in typecheck");
            return false;
    }

    Log::error(expression->location) << "Type check on " << *expression << " failed";

}

bool SimpleTypeChecker::visit_definition(DefinitionBody* definition) {
    return std::visit(overloaded{
        [](Error) { return false; },
        [](Undefined) { return true; },
        [](Expression*) { return true; },
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
    std::span<DefinitionBody* const> extra_definitions) {

    assert(false && "not updated");

    // for (auto scope: scopes) {
    //     for (auto definition: scope->identifiers_in_order_) {
    //         if (!walk_definition(*this, definition))
    //             return false;
    //     }
    // }

    // for (auto definition: extra_definitions) {
    //     if (!walk_definition(*this, definition))
    //         return false;
    // }

    return true;
}

} // namespace Maps
