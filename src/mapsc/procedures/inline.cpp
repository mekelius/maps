#include "inline.hh"

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/compiler_options.hh"

#include "mapsc/types/type.hh"
#include "mapsc/types/function_type.hh"

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/callable.hh"


namespace Maps {

using Log = LogInContext<LogContext::inline_>;

bool inline_call(Expression& expression) {
    #ifndef NDEBUG
    if (CompilerOptions::get(CompilerOption::DEBUG_no_inline) == "true")
        return false;
    #endif
    
    assert(expression.expression_type == ExpressionType::call && 
        "inline_call called with not a call");

    auto [callee, args] = expression.call_value();
    return inline_call(expression, *callee);
}

bool inline_call(Expression& expression, Callable& callable) {
    #ifndef NDEBUG
    if (CompilerOptions::get(CompilerOption::DEBUG_no_inline) == "true")
        return false;
    #endif

    assert(expression.expression_type == ExpressionType::call && 
        "inline_call called with not a call");

    auto [callee, args] = expression.call_value();
    
    if (callable.get_type()->is_impure())
        return false;

    if (args.empty()) {
        Log::debug_extra("Changed nullary call back to a reference", expression.location);
        expression.expression_type = ExpressionType::reference;
        return substitute_value_reference(expression, callable);
    }

    // auto callee_type = dynamic_cast<const FunctionType*>(callee->get_type());
    return false;


    // for (int i = 0; Expression* arg: args) {
        
    // }

    // if (callee_type->is_pure) {
        
    // }
    
}

namespace {

// this should be ran after all the checks are cleared
[[nodiscard]] bool perform_substitution(Expression& expression, Callable& callee) {
    if (auto inner_expression = std::get_if<Expression*>(&callee.body_)) {
        expression = **inner_expression;
        return true;   
    }

    return false;
}

} // anonymous namespace

bool substitute_value_reference(Expression& expression) {
    assert(expression.expression_type == ExpressionType::reference && 
        "substitute_value_reference called with not a reference");

    auto callee = expression.reference_value();
    return substitute_value_reference(expression, *callee);
}

bool substitute_value_reference(Expression& expression, Callable& callee) {
    assert(expression.expression_type == ExpressionType::reference && 
        "substitute_value_reference called with not a reference");

    if (callee.is_undefined()) {
        Log::error("\"" + std::string{callee.name_} + "\" is undefined", expression.location);
        return false;
    }

    // check that the types match, or try to cast
    auto callee_type = callee.get_type();
    auto callee_declared_type = callee.get_declared_type();

    if (callee_declared_type && expression.declared_type) {
        if (**callee_declared_type != **expression.declared_type) {
            Log::warning("Attempting substitution, declared types don't match: " + 
                (*expression.declared_type)->to_string() + " != " + 
                (*callee_declared_type)->to_string(), 
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
        Log::error(
            "incompatible types: " + callee_type->to_string() + " and " + expression.type->to_string(), 
            expression.location);
        return false;
    }

    return perform_substitution(expression, callee);
}

} // namespace Maps
    