#include "concretize.hh"

#include "mapsc/logging.hh"
#include "mapsc/ast/ast_node.hh"
#include "mapsc/types/casts.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/procedures/inline.hh"

using std::get, std::get_if, std::optional, std::nullopt;
using Logging::log_error;

namespace Maps {

bool concretize_call(Expression& call) {
    auto [callee, args] = call.call_value();

    // attempt inline first
    if (inline_call(call, *callee))
        return concretize_expression(call);

    if (!callee->get_type()->is_function()) {
        if (!args.empty())
            return false;

        if (!inline_call(call, *callee))
            return false;
            
        
        //inline it
    }

    // TODO: callable should probably return a function type?
    auto callee_type = dynamic_cast<const FunctionType*>(callee->get_type());

    for (int i = 0; auto arg: args) {
        if (*arg->type == *callee->get_type())

        if (!concretize_expression(*arg))
            return false;
    }

    if (callee_type->is_native() == db_true)
        return true;
    
    if (callee_type->is_castable_to_native() == db_false)
        return false;

    if (callee_type->arity() == 0 && callee_type->is_pure_)
        return inline_call(call, *callee);

    if (call.type->arity() == 1) {
        return false;
    }


    // assuming no inline, we can't do much to the return value
    // 
    // take the return value and 

    return false;
}


bool concretize_reference(Expression& value) {
    if (value.type->is_native() == db_true)
        return true;

    if (value.type->is_castable_to_native() == db_false)
        return false;

    if (!value.type->concretize(value))
        return false;

    return true;
}

bool concretize_value(Expression& value) {
    if (value.type->is_native() == db_true)
        return true;

    if (value.type->is_castable_to_native() == db_false)
        return false;

    if (!value.type->concretize(value))
        return false;

    return true;
}

bool concretize_expression(Expression& expression) {
    switch (expression.expression_type) {
        case ExpressionType::call:
            return concretize_call(expression);
        case ExpressionType::value:
            return concretize_value(expression);
        case ExpressionType::reference:
            return concretize_reference(expression);
        default:
            log_error("Concretizer encountered an expression that was not a value or a call", expression.location);
            return false;
    }
}

} // namespace Maps
