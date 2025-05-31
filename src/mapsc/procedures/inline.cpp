#include "inline.hh"

#include <cassert>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/types/type.hh"

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/definition_body.hh"


namespace Maps {

using Log = LogInContext<LogContext::inline_>;

bool inline_and_substitute(DefinitionBody& definition) {
    return std::visit(overloaded{
        [](auto) { return true; }
    }, definition.body());
}

bool inline_call(Expression& expression) {
    assert(expression.expression_type == ExpressionType::call && 
        "inline_call called with not a call");

    assert(false && "not updated");
    auto [callee, args] = expression.call_value();
    // return inline_call(expression, *callee);
}

bool inline_call(Expression& expression, const DefinitionBody& definition) {
    assert(expression.expression_type == ExpressionType::call && 
        "inline_call called with not a call");

    auto [callee, args] = expression.call_value();
    
    if (definition.get_type()->is_impure())
        return false;

    if (args.empty()) {
        Log::debug_extra("Changed nullary call back to a reference", expression.location);
        expression.expression_type = ExpressionType::reference;
        return substitute_value_reference(expression, definition);
    }

    return false;
}

namespace {

// this should be ran after all the checks are cleared
[[nodiscard]] bool perform_substitution(Expression& expression, const DefinitionBody& callee) {
    auto callee_body = callee.body();
    if (auto inner_expression = std::get_if<Expression*>(&callee_body)) {
        expression = **inner_expression;
        return true;   
    }

    return false;
}

} // anonymous namespace

bool substitute_value_reference(Expression& expression) {
    assert(expression.expression_type == ExpressionType::reference && 
        "substitute_value_reference called with not a reference");

    Log::debug_extra("Substituting " + expression.log_message_string(), expression.location);

    auto callee = expression.reference_value();

    if (!callee->body_) {
        Log::error("Substitution failed", expression.location);
        return false;
    }

    return substitute_value_reference(expression, **callee->body_);
}

bool substitute_value_reference(Expression& expression, const DefinitionBody& callee) {
    assert(expression.expression_type == ExpressionType::reference && 
        "substitute_value_reference called with not a reference");

    // if (callee.is_undefined()) {
    //     Log::error("\"" + callee.name_string() + "\" is undefined", expression.location);
    //     return false;
    // }

    // check that the types match, or try to cast
    auto callee_type = callee.get_type();
    auto callee_declared_type = callee.get_declared_type();

    if (callee_declared_type && expression.declared_type) {
        if (**callee_declared_type != **expression.declared_type) {
            Log::warning("Attempting substitution, declared types don't match: " + 
                (*expression.declared_type)->name_string() + " != " + 
                (*callee_declared_type)->name_string(), 
                expression.location);
            return false;
        }
    }

    if (callee_type->is_pure_function()) {
        // check if what we want is a function
        if (expression.declared_type) {
            if (**expression.declared_type == **callee_declared_type || 
                **expression.declared_type == *callee_type)
                    return perform_substitution(expression, callee);
        }

        if (callee_type->arity() == 0)
            return perform_substitution(expression, callee);

        return false;
    }

    // reject impure functions (maybe we can get llvm to inline them?)
    if (callee_type->is_impure_function()) {
        Log::warning("Impure functions aren't yet inlinable", expression.location);
        return false;
    }

    if (*callee_type != *expression.type) {
        // try to cast
        Log::debug_extra("Cannot inline " + expression.log_message_string() + 
            " due to incompatible types: " + callee_type->name_string() + " and " + 
            expression.type->name_string(), expression.location);
        return false;
    }

    return perform_substitution(expression, callee);
}

} // namespace Maps
    