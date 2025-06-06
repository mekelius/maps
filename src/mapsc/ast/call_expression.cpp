#include "call_expression.hh"

#include "mapsc/source_location.hh"
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

const CallExpressionValue& Expression::call_value() const {
    return std::get<CallExpressionValue>(value);
}

Expression* Expression::partially_applied_minus_value() const {
    return nullptr; // !!!
    // return std::get<>(value);
}

Expression* create_missing_argument(AST_Store& store, const Type* type, SourceLocation location) {
    return store.allocate_expression({ExpressionType::missing_arg, 
        std::monostate{}, type, std::move(location)});
}

optional<Expression*> create_call(CompilationState& state, const DefinitionHeader* callee, 
std::vector<Expression*>&& args, SourceLocation location) {
    for (auto arg: args)
        assert(is_allowed_as_arg(*arg) && "invalid arg passed to Expression::call");

    auto& store = *state.ast_store_;
    auto callee_type = callee->get_type();
    
    if (!callee_type->is_function() && args.size() > 0) {
        Log::error(callee->location()) << *callee << " cannot take arguments, tried giving " << 
            args.size() << Endl;
        return nullopt;
    }

    if (!callee_type->is_function() && args.empty())
        return store.allocate_expression({ExpressionType::call, 
            CallExpressionValue{callee, args}, callee_type, std::move(location)});

    auto [types_ok, is_partial, no_unparsed_args, return_type] = 
        check_and_coerce_args(state, callee, args, location);

    assert(args.size() == callee_type->arity() && 
        "Something went wrong while creating placeholders for missing args");

    if (!types_ok) {
        Log::error(location) << "Creating function call to " << *callee << 
            " failed due to illegal arguments" << Endl;
        return nullopt;
    }

    if (!is_partial)
        return store.allocate_expression({ExpressionType::call, 
            CallExpressionValue{callee, args}, return_type, std::move(location)});

    // TODO: deal with declared types

    return store.allocate_expression({ExpressionType::partial_call, 
        CallExpressionValue{callee, args}, return_type, std::move(location)});
}

std::optional<Expression*> create_call(CompilationState& state, DefinitionBody* callee, 
std::vector<Expression*>&& args, SourceLocation location) {
    return create_call(state, callee->header_, std::move(args), std::move(location));
}

optional<Expression*> create_partial_binop_call(CompilationState& state, const Operator* op, 
Expression* lhs, Expression* rhs, SourceLocation location) {
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
                CallExpressionValue{op, {lhs, rhs}}, partial_return_type, std::move(location)});
    }
    
    assert(rhs->expression_type == ExpressionType::missing_arg && 
        "Expression::partial_binop_call called without a missing argument");
    
    assert(callee_f_type->param_type(1) == rhs->type && 
            "Expression::partial_binop_call called with a non matching lhs type");

    auto partial_return_type = state.types_->get_function_type(
        return_type, std::array{rhs->type}, callee_f_type->is_pure());
    return store.allocate_expression(
            {ExpressionType::partial_binop_call_right, 
                CallExpressionValue{op, {lhs, rhs}}, partial_return_type, std::move(location)});
}

static std::optional<Expression*> partial_binop_call_both(CompilationState& state,
const Operator* lhs, Expression* lambda, const Operator* rhs, SourceLocation location) {
    assert(false && "not implemented");
}

bool convert_to_partial_binop_call_left(CompilationState& state, Expression& expression) {
    if (expression.expression_type != ExpressionType::partially_applied_minus) {
        assert(false && 
           "Expression::convert_to_partial_binop_call_left called on a not partially applied minus");

        Log::compiler_error(expression.location) <<
            "Expression::convert_to_partial_binop_call_left called on a not partially applied minus" 
            << Endl;

        expression.expression_type = ExpressionType::compiler_error;
        return false;
    }

    expression.expression_type = ExpressionType::partial_binop_call_left;

    auto rhs = std::get<Expression*>(expression.value);

    std::vector<Expression*> args{
        create_missing_argument(*state.ast_store_, &Int, expression.location), rhs};

    auto [types_ok, is_partial, no_unparsed_args, return_type] = 
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

        Log::compiler_error(expression.location) <<
            "Expression::convert_to_unary_minus_call called on a not partially applied minus" 
            << Endl;

        expression.expression_type = ExpressionType::compiler_error;
        return false;
    }

    auto arg = std::get<Expression*>(expression.value);
    std::vector<Expression*> args{arg};

    auto [types_ok, is_partial, no_unparsed_args, return_type] = 
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
    assert(expression.type->arity() == 0 && 
        "convert_nullary_reference_to_call called with not a nullary ref");

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
SourceLocation location) {
    return store.allocate_expression({ExpressionType::partially_applied_minus, 
        rhs, &Int, std::move(location)});
}

[[nodiscard]] optional<Expression*> complete_partial_binop_call_left(CompilationState& state, 
Expression& partial_binop_call, Expression& arg) {
    assert(is_partial_call(partial_binop_call) && "invalid call to complete partial binop call");
    assert(is_binop_left(partial_binop_call) && "invalid call to complete partial binop call");

    auto& [callee, args] = partial_binop_call.call_value();

    args.at(0) = &arg;

    auto [types_ok, is_partial, no_unparsed_args, return_type] = 
        check_and_coerce_args(state, callee, args, arg.location);

    assert(types_ok);
    assert(!is_partial);
    assert(no_unparsed_args);
    assert(*args.at(0) == arg);
    
    partial_binop_call.expression_type = ExpressionType::call;

    return &partial_binop_call;
}

} // namespace Maps