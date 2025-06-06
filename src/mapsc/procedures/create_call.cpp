#include "create_call.hh"

#include <string>

#include "mapsc/logging.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/types/function_type.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/call_expression.hh"
#include "mapsc/ast/expression.hh"

#include "mapsc/procedures/update_type.hh"

using std::to_string;

namespace Maps {

using Log = LogInContext<LogContext::type_checks>;

// return values are: <bool success, bool partial, bool is done, const Type* return_type>
std::tuple<bool, bool, bool, const Type*> check_and_coerce_args(CompilationState& state, 
const DefinitionHeader* callee, std::vector<Expression*>& args, const SourceLocation& location) {
    Log::debug_extra(location) << "Processing argument list for a call to " << *callee << Endl;

    bool seen_unparsed_expression = false;

    auto callee_f_type = dynamic_cast<const FunctionType*>(callee->get_type());
    auto param_types = callee_f_type->param_types();
    auto return_type = callee_f_type->return_type();

    if (args.size() > param_types.size()) {
        Log::error(location) << *callee << " takes a maximum of " << param_types.size() << 
            " arguments, tried giving " << args.size() << Endl;
        return {false, false, false, return_type};
    }

    bool missing_args = false;

    // check existing args
    uint i = 0;
    for (auto& arg: args) {
        auto param_type = *callee_f_type->param_type(i);

        if (arg->expression_type == ExpressionType::layer2_expression) {
            Log::debug_extra(arg->location) << "Argument " << i << " was unparsed" << Endl;
            seen_unparsed_expression = true;
            arg->type = param_type; // NOTE: not handled yet in layer2
            i++;
            continue;
        }

        if (arg->expression_type == ExpressionType::missing_arg)
            missing_args = true;

        // handle unkown type args
        if (arg->type->is_unknown()) {
            Log::debug_extra(arg->location) << "Argument " << i << 
                " (" << *arg << ") has an unkown type, trying to update/deduce..." << Endl;

            if (!update_type(*arg)) {
                Log::error(arg->location) << "Updating type failed" << Endl;
                return {false, false, false, &ErrorType};
            }
            
            Log::debug_extra(arg->location) << 
                "Updated type of " << *arg << " to " << *arg->type << Endl;
        }

        if (*arg->type == *param_type) {
            Log::debug_extra(arg->location) << "Argument " << i << " matched parameter type" << Endl;
            i++;
            continue;
        }

        Log::debug_extra(arg->location) << 
            "Attempting to cast argument " << i << " to " << *param_type << Endl;
        
        auto new_arg = arg->cast_to(state, param_type);
        if (!new_arg)
            return {false, false, false, return_type};

        arg = *new_arg;

        i++;
        continue;
    }

    // fill in the missing args, if any
    while (i < param_types.size()) {
        missing_args = true;
        args.push_back(create_missing_argument(*state.ast_store_, *callee_f_type->param_type(i), 
            location));
        Log::debug_extra(location) << 
            "Inserted missing argument placeholder as argument " << i << Endl;
        i++;
    }

    if (missing_args) {
        std::vector<const Type*> missing_arg_types{};

        for (size_t i = args.size(); i < param_types.size(); i++) {
            auto param_type = *callee_f_type->param_type(i);
            missing_arg_types.push_back(param_type);
            args.push_back(create_missing_argument(*state.ast_store_, param_type, location));
        }

        auto partial_return_type = state.types_->get_function_type(
            return_type, missing_arg_types, callee_f_type->is_pure());

        return {true, true, !seen_unparsed_expression, partial_return_type};
    }

    return {true, false, !seen_unparsed_expression, return_type};
}

} // namespace Maps