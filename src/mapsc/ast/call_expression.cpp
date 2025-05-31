#include "call_expression.hh"

#include "mapsc/source.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/ast/expression_properties.hh"
#include "mapsc/procedures/create_call.hh"

using std::optional, std::nullopt, std::to_string;

namespace Maps {

using Log = LogNoContext;

CallExpressionValue& Expression::call_value() {
    return std::get<CallExpressionValue>(value);
}

Expression* Expression::partially_applied_minus_arg_value() const {
    return nullptr; // !!!
    // return std::get<>(value);
}

Expression* create_missing_argument(AST_Store& store, const Type* type, 
    const SourceLocation& location) {
    
    return store.allocate_expression({ExpressionType::missing_arg, std::monostate{}, type, 
        location});
}

optional<Expression*> create_call(CompilationState& state, 
    DefinitionHeader* callee, std::vector<Expression*>&& args, const SourceLocation& location) {

    for (auto arg: args)
        assert(is_allowed_as_arg(*arg) && "invalid arg passed to Expression::call");

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

std::optional<Expression*> create_call(CompilationState& state, 
    DefinitionBody* callee, std::vector<Expression*>&& args, const SourceLocation& location) {

    return create_call(state, callee->header_, std::move(args), location);
}


optional<Expression*> create_partial_binop_call(CompilationState& state, 
    Operator* op, Expression* lhs, Expression* rhs, const SourceLocation& location) {

    auto& store = *state.ast_store_;
    auto callee_type = op->get_type();
    
    assert(op->is_operator() && 
        "Expression::partial_binop_call called with not an operator");
    
    assert(op->is_binary() && 
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
                CallExpressionValue{op, {lhs, rhs}}, partial_return_type, 
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
                CallExpressionValue{op, {lhs, rhs}}, partial_return_type, 
                location});
}

static std::optional<Expression*> partial_binop_call_both(CompilationState& state,
    DefinitionHeader* lhs, Expression* lambda, DefinitionHeader* rhs, const SourceLocation& location) {

    assert(false && "not implemented");
}


bool convert_to_partial_binop_minus_call_left(CompilationState& state, Expression& expression) {
    if (expression.expression_type != ExpressionType::partially_applied_minus) {
        assert(false && 
           "Expression::convert_to_partial_binop_call_left called on a not partially applied minus");

        Log::compiler_error(
            "Expression::convert_to_partial_binop_call_left called on a not partially applied minus", 
            expression.location);

        expression.expression_type = ExpressionType::compiler_error;
        return false;
    }

    expression.expression_type = ExpressionType::partial_binop_call_left;

    auto rhs = std::get<Expression*>(expression.value);

    std::vector<Expression*> args{
        create_missing_argument(*state.ast_store_, &Int, expression.location), rhs};

    auto [types_ok, is_partial, work_to_be_done, return_type] = 
        check_and_coerce_args(state, &binary_minus_Int, args, expression.location);

    if (!types_ok)
        return false;
    
    expression.value = CallExpressionValue(&binary_minus_Int, args);

    // value = CallExpressionValue(&binary_minus_Int, 
    //     {Expression::missing_argument(store, &Int, location), rhs});
    expression.type = return_type;

    return true;
}

bool convert_to_unary_minus_call(CompilationState& state, Expression& expression) {
    if (expression.expression_type != ExpressionType::partially_applied_minus) {
        assert(false && 
           "Expression::convert_to_unary_minus_call called on a not partially applied minus");

        Log::compiler_error(
            "Expression::convert_to_unary_minus_call called on a not partially applied minus", 
            expression.location);

        expression.expression_type = ExpressionType::compiler_error;
        return false;
    }

    auto arg = std::get<Expression*>(expression.value);
    std::vector<Expression*> args{arg};

    auto [types_ok, is_partial, work_to_be_done, return_type] = 
        check_and_coerce_args(state, &unary_minus_Int, args, expression.location);

    assert(!is_partial);

    if (!types_ok)
        return false;

    expression.expression_type = ExpressionType::call;
    expression.value = CallExpressionValue(&unary_minus_Int, args);
    expression.type = return_type;

    return true;
}

void convert_nullary_reference_to_call(Expression& expression) {
    assert(expression.expression_type == ExpressionType::reference && 
        "convert_nullary_reference_to_call called with not a ref");
    assert(expression.type->arity() == 0 && "convert_nullary_reference_to_call called with not a nullary ref");

    expression.expression_type = ExpressionType::call;
    auto callee = expression.reference_value();

    // assert(!callee->is_undefined() && !callee->is_empty());

    expression.value = CallExpressionValue{callee, {}};
}

bool convert_partially_applied_minus_to_arg(CompilationState& state, Expression& expression,
    const Type* param_type) {

    assert(expression.expression_type == ExpressionType::partially_applied_minus && 
        "convert_partially_applied_minus_to_arg called with not a partially applied minus");

    if (param_type->arity() == 0)
        return convert_to_unary_minus_call(state, expression);

    convert_to_partial_call(expression);
    return true;
}


void convert_to_partial_call(Expression& expression) {
    expression.expression_type = ExpressionType::partial_call;
}

Expression* create_partially_applied_minus(AST_Store& store, Expression* rhs, 
    const SourceLocation& location) {

    return store.allocate_expression(
        {ExpressionType::partially_applied_minus, rhs, &Int, location});
}

} // namespace Maps