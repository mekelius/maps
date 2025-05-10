#include "concretize.hh"

#include <cassert>

#include "mapsc/logging.hh"
#include "mapsc/ast/ast_node.hh"
#include "mapsc/types/casts.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/procedures/inline.hh"

using std::get, std::get_if, std::optional, std::nullopt;
using Maps::GlobalLogger::log_error, Maps::GlobalLogger::log_info;

namespace Maps {

bool concretize_call(Expression& call) {
    auto [callee, args] = call.call_value();

    // attempt inline first
    if (inline_call(call, *callee))
        return concretize(call);

    // TODO: callable should probably return a function type?
    auto callee_type = dynamic_cast<const FunctionType*>(callee->get_type());

    // if it's not a function, it should have been inlinable?
    if (!callee_type) {
        log_info("Concretizing a call failed", MessageType::post_parse_debug, call.location);
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

    for (int i = 0; auto arg: args) {
        auto param_type = callee_type->arg_types_.at(i);

        if (*arg->type == *param_type)
            continue;

        if (!arg->type->cast_to(param_type, *arg))
            return false;
    }

    // handle return type
    auto return_type = callee_type->return_type_;

    if (return_type->is_native() == db_true)
        return true;

    if (return_type->is_castable_to_native() == db_false)
        return false;

    if (return_type->is_castable_to_native() == db_true) {
        assert(false && "concretizing return values not implemented");
    }
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
    if (value.type->is_native() == db_true)
        return true;

    if (value.type->is_castable_to_native() == db_false)
        return false;

    if (!value.type->concretize(value))
        return false;

    return true;
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
        default:
            log_error("Concretizer encountered an expression that was not a value or a call", expression.location);
            return false;
    }
}

} // namespace Maps
