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

using std::get, std::get_if, std::optional, std::nullopt;
using Maps::GlobalLogger::log_error;

namespace Maps {

bool try_to_coerce(Expression& expression, const Type* type) {
    return expression.type->cast_to(type, expression);
}

bool handle_declared_type(Expression& expression, const Type* declared_type) {
    return try_to_coerce(expression, declared_type);
}

bool SimpleTypeChecker::visit_expression(Expression* expression) {
    if (expression->declared_type && **expression->declared_type != *expression->type)
        return handle_declared_type(*expression, *expression->declared_type);

    if (concretize(*expression))
        return true;

    log_error("Found a non-reduced type: " + static_cast<std::string>(expression->type->name()), 
        expression->location);

    return false;
}

// the first return value signals whether a coercion was performed,
// the second one whether a return value was found
std::pair<bool, bool> coerce_block_return_type(Statement& statement, const Type* wanted_type) {
    std::vector<Statement*> block = std::get<Block>(statement.value);
    
    for (auto substatement: block) {
        switch (substatement->statement_type) {
            // TODO: handle conditional returns
            case StatementType::block: {
                auto [coerced, had_return] = coerce_block_return_type(*substatement, wanted_type);
                if (had_return)
                    return {coerced, had_return};
                    
                continue;
            }
            case StatementType::return_:
                return {
                    handle_declared_type(*std::get<Expression*>(substatement->value), wanted_type), 
                    true
                };

            default:
                continue;
        }
    }

    return {false, false};
}

// Gets the return type of a statement, trying to coerce into the wanted type if given
bool coerce_return_type(Statement& statement, const Type* wanted_type) {
    switch (statement.statement_type) {
        case StatementType::block:
            return coerce_block_return_type(statement, wanted_type).first;
        
        case StatementType::expression_statement:
        case StatementType::return_:
            return handle_declared_type(*std::get<Expression*>(statement.value), wanted_type);
        default:
            return false;
    }
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
