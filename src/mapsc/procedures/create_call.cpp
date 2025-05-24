#include "create_call.hh"

#include <string>

#include "mapsc/ast/definition.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/logging.hh"

using std::to_string;

namespace Maps {

using Log = LogInContext<LogContext::type_checks>;

// return values are: <bool success, bool partial>
std::pair<bool, bool> check_and_coerce_args(AST_Store& store, const Definition* callee, 
    std::vector<Expression*>& args, const SourceLocation& location) {

    auto callee_f_type = dynamic_cast<const FunctionType*>(callee->get_type());
    auto param_types = callee_f_type->param_types();
    auto return_type = callee_f_type->return_type();

    if (args.size() > param_types.size()) {
        Log::error(callee->name_string() + " takes a maximum of " + to_string(param_types.size()) + 
            " arguments, tried giving " + to_string(args.size()), location);
        return {false, false};
    }

    bool missing_args = false;

    // check existing args
    uint i = 0;
    for (auto arg: args) {
        auto param_type = *callee_f_type->param_type(i);
        i++;

        if (arg->expression_type == ExpressionType::missing_arg)
            missing_args = true;

        if (arg->type == param_type)
            continue;

        if (*arg->type == Hole)
            arg->type = param_type;

        // copy and try to cast
        // if (arg->)
        //     if reference copy to value
        //     if (concrete) call, try to inline,
        //         if not, add a runtime cast

        //     if fails, fail
    }

    // fill in the missing args, if any
    while (i < param_types.size()) {
        args.push_back(Expression::missing_argument(store, *callee_f_type->param_type(i), location));
    }

    for (auto arg: args) {
    if (arg->expression_type == ExpressionType::missing_arg)
        missing_args = true;
    }

    return {true, missing_args};
}

} // namespace Maps