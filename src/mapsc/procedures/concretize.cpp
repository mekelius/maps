#include "concretize.hh"

#include <cassert>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"


#include "mapsc/ast/expression.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/types/type.hh"
#include "mapsc/types/function_type.hh"

#include "mapsc/procedures/inline.hh"

using std::get, std::get_if, std::optional, std::nullopt;


namespace Maps {

using Log = LogInContext<LogContext::concretize>;

bool concretize_call(Expression& call) {
    auto [callee, args] = call.call_value();

    // attempt inline first
    if (inline_call(call, *callee))
        return concretize(call);

    // TODO: definition should probably return a function type?
    auto callee_type = dynamic_cast<const FunctionType*>(callee->get_type());

    // if it's not a function, it should have been inlinable?
    if (!callee_type) {
        Log::warning("Concretizing a call failed", call.location);
        return false;
    }

    if (call.declared_type) {
        if (**call.declared_type != *callee_type) {
            assert(false && "mismathing declared type not implemented in concretize call");
        }

        if (!callee->get_type()->is_function()) {
            if (!args.empty())
                return false;
    
            // ???
        }
    }

    // handle args
    assert(args.size() <= callee_type->arity() && "call expression has too many args");
    assert(args.size() == callee_type->arity() && "partial calls not implemented in concretize_call");

    for (int i = 0; auto param_type: callee_type->param_types()) {
        auto arg = args.at(i++);

        if (*arg->type == *param_type)
            continue;

        if (arg->is_constant_value() && !arg->type->cast_to(param_type, *arg))
            return false;

        if (!concretize(*arg))
            return false;

        if (*arg->type != *param_type) {
            Log::error(arg->log_message_string() + 
                " does not match parameter type: " + param_type->to_string(),
                arg->location);
            return false;
        }
    }

    // handle return type
    auto return_type = callee_type->return_type();

    if (return_type->is_concrete())
        return true;
    
    assert(false && "concretizing return values not implemented");
    // create a call to cast function here

    return false;
}

bool concretize_reference(Expression& value) {
    if (!value.declared_type)
        return substitute_value_reference(value);

    if (!value.type->concretize(value))
        return false;

    return true;
}

bool concretize_value(Expression& value) {
    return(value.type->concretize(value));
}

bool concretize(Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::call:
            return concretize_call(expression);

        case ExpressionType::string_literal:
            return true;

        case ExpressionType::numeric_literal:
        case ExpressionType::value:
            return concretize_value(expression);

        case ExpressionType::reference:
            return concretize_reference(expression);

        case ExpressionType::partially_applied_minus:
            expression.convert_to_unary_minus_call();
            return concretize_call(expression);

        default:
            Log::error("Concretizer encountered an expression that was not a value or a call", 
                expression.location);
            return false;
    }
}

} // namespace Maps
