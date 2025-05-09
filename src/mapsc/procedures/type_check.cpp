#include "type_check.hh"

#include <variant>
#include <cassert>
#include <optional>

#include "common/std_visit_helper.hh"
#include "mapsc/logging.hh"
#include "mapsc/types/casts.hh"
#include "mapsc/ast/ast_node.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/procedures/concretize.hh"
#include "mapsc/procedures/coerce_type.hh"

using std::get, std::get_if, std::optional, std::nullopt;
using Maps::GlobalLogger::log_error;

namespace Maps {

bool SimpleTypeChecker::visit_expression(Expression* expression) {
    if (expression->declared_type && **expression->declared_type != *expression->type)
        return handle_declared_type(*expression, *expression->declared_type);

    if (concretize(*expression))
        return true;

    log_error("Found a non-reduced type: " + static_cast<std::string>(expression->type->name()), 
        expression->location);

    return false;
}

bool SimpleTypeChecker::visit_callable(Callable* callable) {
    return std::visit(overloaded{
        [](std::monostate&) { return true; },
        [](Builtin*) { return true; },
        [](Expression*) { return true; },
        [callable](Statement* statement) {                        
            auto type = callable->get_type();
            auto declared_type = callable->get_declared_type();

            if (declared_type) {
                if (type && *type != **declared_type) {
                    assert(false && "in visit_callable: declared type mismatch, handling not implemented");
                    return false;
                }
            }

            // if (*type == Hole)
                // get return type and set it

            return coerce_return_type(*statement, type);
        }
    }, callable->body);
}

// Note, statements shouldn't mess with contained expressions, since they will be visited
bool SimpleTypeChecker::visit_statement(Statement* statement) { 
    return true; 
}

bool SimpleTypeChecker::run(AST_Store& ast) {
    // return ast.walk_tree(*this);
    // !!! ignoring failures
    ast.walk_tree(*this);
    return true;
}

} // namespace Maps
