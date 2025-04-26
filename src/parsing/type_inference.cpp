#include "type_inference.hh"

#include <variant>
#include <cassert>
#include <optional>

#include "../logging.hh"

using std::get, std::get_if, std::optional, std::nullopt;
using Logging::log_error;

namespace Maps {

bool has_native_representation(const Type* type) {    
    if (type->is_native())
        return true;

    if (!type->is_function())
        return false;

    auto function_type = type->function_type();

    if (!has_native_representation(function_type->return_type))
        return false;

    for (auto arg: function_type->arg_types) {
        if (!has_native_representation(arg))
            return false;
    }

    return true;
}

optional<const Type*> handle_declared_type(const Type* actual_type, const Type* declared_type) {
    
    if (*actual_type == NumberLiteral && *declared_type == Float) {

    }    
    return nullopt;
}

void SimpleTypeChecker::visit_expression(Expression* expression) {
    if (expression->declared_type && **expression->declared_type != *expression->type) {
        auto deduced_type = handle_declared_type(expression->type, *expression->declared_type);

        if (!deduced_type)
            return fail();

        if (!has_native_representation(*deduced_type))
            return fail();

        expression->type = *deduced_type;
        return;
    }

    if (has_native_representation(expression->type))
        return;

    log_error(expression->location, "Found a non-reduced type: " + static_cast<std::string>(expression->type->name()));

    return fail();
}

void SimpleTypeChecker::visit_callable(Callable* callable) {
    if (callable->get_declared_type() && **callable->get_declared_type() != *callable->get_type()) {
        auto deduced_type = handle_declared_type(callable->get_type(), *callable->get_declared_type());

        if (!deduced_type)
            return fail();

        if (!has_native_representation(*deduced_type))
            return fail();

        callable->set_type(**deduced_type);
        return;
    }

    if (!has_native_representation(callable->get_type()))
        return fail();
}

// Note, statements shouldn't mess with contained expressions, since they will be visited
void SimpleTypeChecker::visit_statement(Statement* statement) {}

bool SimpleTypeChecker::run(AST& ast) {
    ast.visit_nodes(*this);

    return success;
}


bool handle_callable(AST& ast, Callable& callable) {
    assert(false && "not implemented");
    const Type* type = callable.get_type();

    if (!type->is_native()) {

    }

    if (Expression* const* expression = get_if<Expression*>(&callable.body)) {
        // handle_expression();
    }

    if (Statement* const* expression = get_if<Statement*>(&callable.body)) {

    }

    return false;
}

// first attempt
bool infer_types(AST& ast) {
    assert(false && "not implemented");

    for (auto [_1, callable]: ast.globals_->identifiers_in_order_) {
        if (!handle_callable(ast, *callable)) {

        }
    }

    return true;
}

} // namespace Maps
