#include "type_checking.hh"

#include <variant>
#include <cassert>
#include <optional>

#include "mapsc/logging.hh"

#include "mapsc/types/casts.hh"

using std::get, std::get_if, std::optional, std::nullopt;
using Logging::log_error;

namespace Maps {

optional<const Type*> handle_declared_type(const Type* actual_type, const Type* declared_type) {
    return nullopt;
}

bool TypeConcretizer::concretize_expression(Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::call:
            return concretize_call(expression);
        case ExpressionType::value:
            return concretize_value(expression);
        default:
            return false;
    }
}

bool TypeConcretizer::concretize_call(Expression& call) {
    if (call.type->is_native() == db_true)
        return true;
    
    if (call.type->is_castable_to_native() == db_false)
        return false;

    return false;
}

bool TypeConcretizer::concretize_value(Expression& value) {
    if (value.type->is_native() == db_true)
        return true;

    if (value.type->is_castable_to_native() == db_false)
        return false;

    if (!value.type->concretize(value))
        return false;

    return true;
}

bool SimpleTypeChecker::visit_expression(Expression* expression) {
    if (expression->declared_type && **expression->declared_type != *expression->type) {
        auto deduced_type = handle_declared_type(expression->type, *expression->declared_type);

        if (!deduced_type)
            return false;

        if ((*deduced_type)->is_castable_to_native() == db_false)
            return false;

        expression->type = *deduced_type;
        return true;
    }

    if (expression->type->is_native() == db_true)
        return true;

    // first try to cast it into an int, then a float
    if (*expression->type == NumberLiteral) {
        if (expression->type->cast_to(&Int, *expression))
            return true;

        if (expression->type->cast_to(&Float, *expression))
            return true;

        log_error(expression->string_value() + " is not a valid number", expression->location);
        return false;
    }

    log_error("Found a non-reduced type: " + static_cast<std::string>(expression->type->name()), 
        expression->location);

    return false;
}

bool SimpleTypeChecker::visit_callable(Callable* callable) {
    if (std::holds_alternative<Expression*>(callable->body))
        return true;

    // TODO: how to get to return type?
    assert(false && "type check for statement callables not implemented");
    return false;

    auto type = callable->get_type();
    auto declared_type = callable->get_declared_type();

    if (*type == Hole) {
    }

    if (declared_type && **declared_type != *type) {
        auto deduced_type = handle_declared_type(callable->get_type(), *callable->get_declared_type());

        if (!deduced_type)
            return false;

        if ((*deduced_type)->is_castable_to_native() == db_false)
            return false;

        callable->set_type(**deduced_type);
        return true;
    }

    return (callable->get_type()->is_castable_to_native() == db_true);
}

// Note, statements shouldn't mess with contained expressions, since they will be visited
bool SimpleTypeChecker::visit_statement(Statement* statement) {return true;}

bool SimpleTypeChecker::run(AST_Store& ast) {
    // return ast.walk_tree(*this);
    // !!! ignoring failures
    ast.walk_tree(*this);
    return true;
}

} // namespace Maps
