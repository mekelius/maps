#include "../expression.hh"

#include "mapsc/source.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/procedures/create_call.hh"

using std::optional, std::nullopt, std::to_string;

namespace Maps {

using Log = LogNoContext;

bool Expression::is_partial_call() const {
    switch (expression_type) {
        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_call:
            return true;

        case ExpressionType::call:{
            auto [callee, args] = std::get<CallExpressionValue>(value);

            if (args.size() < callee->get_type()->arity())
                return true;

            for (auto arg: args) {
                if (arg->expression_type == ExpressionType::missing_arg)
                    return true;
            }

            return false;
        }
        default:
            return false;
    }
}

CallExpressionValue& Expression::call_value() {
    return std::get<CallExpressionValue>(value);
}

Expression* Expression::missing_argument(AST_Store& store, const Type* type, 
    const SourceLocation& location) {
    
    return store.allocate_expression({ExpressionType::missing_arg, std::monostate{}, type, 
        location});
}

optional<Expression*> Expression::call(CompilationState& state, 
    Definition* callee, std::vector<Expression*>&& args, const SourceLocation& location) {

    auto& store = *state.ast_store_;
    auto callee_type = callee->get_type();
    
    if (!callee_type->is_function() && args.size() > 0) {
        Log::error(callee->name_string() + 
            " cannot take arguments, tried giving " + to_string(args.size()), 
            callee->location());
        return nullopt;
    }

    if (!callee_type->is_function() && args.empty())
        return store.allocate_expression(
            {ExpressionType::call, CallExpressionValue{callee, args}, callee_type, location});

    auto [types_ok, is_partial, work_to_be_done, return_type] = 
        check_and_coerce_args(state, callee, args, location);

    assert(args.size() == callee_type->arity() && 
        "Something went wrong while creating placeholders for missing args");

    if (!types_ok) {
        Log::error("Creating function call to " + callee->name_string() + 
            " failed due to illegal arguments", location);
        return nullopt;
    }

    if (!is_partial)
        return store.allocate_expression(
            {ExpressionType::call, CallExpressionValue{callee, args}, return_type, location});

    // TODO: deal with declared types

    return store.allocate_expression(
        {ExpressionType::partial_call, CallExpressionValue{callee, args}, 
            return_type, location});
}

optional<Expression*> Expression::partial_binop_call(CompilationState& state, 
    Definition* definition, Expression* lhs, Expression* rhs, const SourceLocation& location) {

    auto& store = *state.ast_store_;
    auto callee_type = definition->get_type();
    
    assert(definition->is_operator() && 
        "Expression::partial_binop_call called with not an operator");
    
    assert(dynamic_cast<Operator*>(definition)->is_binary() && 
        "Expression::partial_binop_call called with not a binary operator");

    auto callee_f_type = dynamic_cast<const FunctionType*>(callee_type);
    auto return_type = callee_f_type->return_type();

    // TODO: deal with declared types

    if (lhs->expression_type == ExpressionType::missing_arg) {
        assert(callee_f_type->param_type(0) == lhs->type && 
            "Expression::partial_binop_call called with a non matching lhs type");
        
        auto partial_return_type = state.types_->get_function_type(
            return_type, std::array{lhs->type}, callee_f_type->is_pure());

        return store.allocate_expression(
            {ExpressionType::partial_binop_call_left, 
                CallExpressionValue{definition, {lhs, rhs}}, partial_return_type, 
                location});
    }
    
    assert(rhs->expression_type == ExpressionType::missing_arg && 
        "Expression::partial_binop_call called without a missing argument");
    
    assert(callee_f_type->param_type(1) == rhs->type && 
            "Expression::partial_binop_call called with a non matching lhs type");

    auto partial_return_type = state.types_->get_function_type(
        return_type, std::array{rhs->type}, callee_f_type->is_pure());
    return store.allocate_expression(
            {ExpressionType::partial_binop_call_right, 
                CallExpressionValue{definition, {lhs, rhs}}, partial_return_type, 
                location});
}

static std::optional<Expression*> partial_binop_call_both(CompilationState& state,
    Definition* lhs, Expression* lambda, Definition* rhs, const SourceLocation& location) {

    assert(false && "not implemented");
}


void Expression::convert_to_partial_binop_minus_call_left(AST_Store& store) {
    if (expression_type != ExpressionType::partially_applied_minus) {
        assert(false && 
           "Expression::convert_to_partial_binop_call_left called on a not partially applied minus");

        Log::compiler_error(
            "Expression::convert_to_partial_binop_call_left called on a not partially applied minus", 
            location);

        expression_type = ExpressionType::compiler_error;
        return;
    }

    expression_type = ExpressionType::partial_binop_call_left;

    auto rhs = std::get<Expression*>(value);
    value = CallExpressionValue(&binary_minus_Int, 
        {Expression::missing_argument(store, &Int, location), rhs});
    type = &Int_to_Int;
}

void Expression::convert_to_unary_minus_call() {
    if (expression_type != ExpressionType::partially_applied_minus) {
        assert(false && 
           "Expression::convert_to_unary_minus_call called on a not partially applied minus");

        Log::compiler_error(
            "Expression::convert_to_unary_minus_call called on a not partially applied minus", 
            location);

        expression_type = ExpressionType::compiler_error;
        return;
    }

    expression_type = ExpressionType::call;
    auto arg = std::get<Expression*>(value);
    value = CallExpressionValue(&unary_minus_Int, {arg});
    type = &Int;
}

// do we even have to do anything?
void Expression::convert_to_partial_call() {
    return;
}

Expression* Expression::partially_applied_minus(AST_Store& store, Expression* rhs, 
    const SourceLocation& location) {

    return store.allocate_expression(
        {ExpressionType::partially_applied_minus, rhs, &Int, location});
}

} // namespace Maps