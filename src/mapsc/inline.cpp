#include "inline.hh"

#include <cassert>

#include "mapsc/logging.hh"
#include "mapsc/ast/ast_node.hh"
#include "mapsc/types/function_type.hh"

using Logging::log_error;

namespace Maps {

bool inline_call(Expression& expression) {
    assert(expression.expression_type == ExpressionType::call && 
        "inline_call called with not a call");

    auto [callee, args] = expression.call_value();
    return inline_call(expression, *callee);
}

bool inline_call(Expression& expression, Callable& callable) {
    assert(expression.expression_type == ExpressionType::call && 
        "inline_call called with not a call");

    return false;
    auto [callee, args] = expression.call_value();

    
    // if (!callee->get_type()->is_function()) {
    //     if (!args.empty()) {

    //     }
    //     return false
    // }

    // auto callee_type = dynamic_cast<const FunctionType*>(callee->get_type());


    // for (int i = 0; Expression* arg: args) {
        
    // }

    // if (callee_type->is_pure) {
        
    // }
    
}

bool substitute_value_reference(Expression& expression) {
    assert(expression.expression_type == ExpressionType::reference && 
        "inline_call called with not a reference");

    auto callee = expression.reference_value();
    return substitute_value_reference(expression, *callee);
}

bool substitute_value_reference(Expression& expression, Callable& callee) {
    assert(expression.expression_type == ExpressionType::reference && 
        "inline_call called with not a reference");

    // check that the callable has a value
    // ??? This should be done centrally?

    if (callee.is_undefined()) {
        log_error("\"" + callee.name + "\" is undefined", expression.location);
        return false;
    }


    // check that the types match, or try to cast
    auto callee_type = callee.get_type();

    if (*callee_type != *expression.type) {
        // try to cast
        log_error("incompatible types: " + callee_type->to_string() + " and " + expression.type->to_string(), 
            expression.location);
        return false;
    }

    if ( = std::get_if<Expression>()) {

    }

    // substitute value
    expression.expression_type = ExpressionType::value;
    expression.value = std::get<>(callee.body);
}

} // namespace Maps
    