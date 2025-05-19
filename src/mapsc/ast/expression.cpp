#include "expression.hh"

#include <memory>
#include <span>
#include <cassert>
#include <sstream>

#include "mapsc/logging.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/callable.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/procedures/reverse_parse.hh"


using std::optional, std::nullopt, std::to_string;

namespace Maps {

using Log = LogNoContext;

// ----- EXPRESSION -----

std::vector<Expression*>& Expression::terms() {
    return std::get<TermedExpressionValue>(value).terms;
}

const std::vector<Expression*>& Expression::terms() const {
    return std::get<TermedExpressionValue>(value).terms;
}


CallExpressionValue& Expression::call_value() {
    return std::get<CallExpressionValue>(value);
}

Callable* Expression::reference_value() const {
    return std::get<Callable*>(value);
}

Callable* Expression::operator_reference_value() const {
    return std::get<Callable*>(value);
}

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

bool Expression::is_reduced_value() const {
    switch (expression_type) {
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
        case ExpressionType::reference:
        case ExpressionType::call:
        case ExpressionType::value:
            return true;

        default:
            return false;
    }
}

void Expression::mark_not_type_declaration() {
    if (expression_type != ExpressionType::termed_expression)
        return;

    std::get<TermedExpressionValue>(value).is_type_declaration = DeferredBool::false_;
}

DeferredBool Expression::is_type_declaration() {
    switch (expression_type) {
        case ExpressionType::termed_expression:
            return std::get<TermedExpressionValue>(value).is_type_declaration;

        case ExpressionType::type_identifier:
        case ExpressionType::type_construct:
        case ExpressionType::type_reference:
        case ExpressionType::type_constructor_reference:
            return DeferredBool::true_;

        default:
            return DeferredBool::false_;
    }
}

// TODO: clean this up
std::string Expression::string_value() const {
    if (std::holds_alternative<Callable*>(value)) {
        // !!! this will cause crashes when lambdas come in
        return std::get<Callable*>(value)->to_string();
    }
    return std::get<std::string>(value);
}

bool Expression::is_literal() const {
    switch (expression_type) {
        case ExpressionType::numeric_literal:
        case ExpressionType::string_literal:
            return true;

        default:
            return false;
    }
}

bool Expression::is_illegal() const {
    switch (expression_type) {
        case ExpressionType::deleted:
        case ExpressionType::not_implemented:
        case ExpressionType::syntax_error:
            return true;

        default:
            return false;
    }
}

bool Expression::is_reference() const {
    switch (expression_type) {
        case ExpressionType::reference:
        case ExpressionType::type_reference:
        case ExpressionType::prefix_operator_reference:
        case ExpressionType::postfix_operator_reference:
        case ExpressionType::binary_operator_reference:
        case ExpressionType::type_operator_reference:
        case ExpressionType::type_constructor_reference:
            return true;

        default:
            return false;
    }
}

bool Expression::is_identifier() const {
    switch (expression_type) {
        case ExpressionType::identifier:
        case ExpressionType::type_identifier:
        case ExpressionType::operator_identifier:
        case ExpressionType::type_operator_identifier:
            return true;

        default:
            return false;
    }
}

bool Expression::is_ok_in_layer2() const {
    return !(is_identifier() || is_illegal() || expression_type == ExpressionType::type_operator_reference);
}

bool Expression::is_ok_in_codegen() const {
    switch (expression_type) {
        case ExpressionType::reference:
        case ExpressionType::call:
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
            return true;

        default:
            return false;
    }
}

bool Expression::is_castable_expression() const {
    return true;
}

bool Expression::is_allowed_in_type_declaration() const {
    switch (expression_type) {
        case ExpressionType::termed_expression:
            return std::get<TermedExpressionValue>(value).is_type_declaration != 
                DeferredBool::false_;

        case ExpressionType::type_argument:
        case ExpressionType::type_construct:
        case ExpressionType::type_operator_identifier:
        case ExpressionType::type_operator_reference:
        case ExpressionType::type_constructor_reference:
        case ExpressionType::type_identifier:
        case ExpressionType::type_reference:
        case ExpressionType::type_field_name:
            return true;

        case ExpressionType::identifier:
            return true;

        default:
            return false;
    }
}

bool Expression::is_constant_value() const {
    switch (expression_type) {
        case ExpressionType::value:
        case ExpressionType::numeric_literal:
        case ExpressionType::string_literal:
            return true;

        default:
            return false;   
    }
}

std::string Expression::log_message_string() const {
    switch (expression_type) {
        case ExpressionType::type_construct:
            return "type argument (TODO pretty print)";
            // TOOD:
            // return type_argument_value().to_string();

        case ExpressionType::type_argument:
            return "type argument (TODO pretty print)";
            // TOOD:
            // return type_argument_value().to_string();

        case ExpressionType::termed_expression:
            return std::get<TermedExpressionValue>(value).to_string();

        case ExpressionType::string_literal:
            return "string literal \"" + string_value() + "\"";

        case ExpressionType::numeric_literal:
            return "numeric literal +" + string_value();
    
        case ExpressionType::value:
            return "value expression of type " + type->to_string();
        
        case ExpressionType::identifier:
            return "identifier " + string_value();

        case ExpressionType::operator_identifier:
            return "operator " + string_value();
        case ExpressionType::type_operator_identifier:
            return "type operator " + string_value();
        case ExpressionType::type_identifier:
            return "type identifier " + string_value();

        case ExpressionType::reference:
            return "reference to " + reference_value()->to_string();
        case ExpressionType::type_reference:
            return "reference to type " + reference_value()->to_string();
        case ExpressionType::binary_operator_reference:
        case ExpressionType::prefix_operator_reference:
        case ExpressionType::postfix_operator_reference:
            return "operator " + reference_value()->to_string();
        case ExpressionType::type_operator_reference:
            return "type operator " + reference_value()->to_string();
        case ExpressionType::type_constructor_reference:
            return "reference to type constructor " + reference_value()->to_string();
   
        case ExpressionType::type_field_name:
            return "named field" + string_value();

        case ExpressionType::syntax_error:
            return "broken expession (syntax error)";
        
        case ExpressionType::compiler_error:
            return "broken expression (compiler error)";

        case ExpressionType::not_implemented:
            return "nonimplemented expression";

        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_call:
        case ExpressionType::call: {
            std::stringstream output{};
            ReverseParser{&output} << "Call expression " << *this;
            return output.str();
        }

        case ExpressionType::missing_arg:
            return "incomplete partial application missing argument of type " + type->to_string();

        case ExpressionType::deleted:
            return "deleted expession";

        case ExpressionType::minus_sign:
            return "-";

        case ExpressionType::partially_applied_minus:
            return "-( " + std::get<Expression*>(value)->log_message_string() + " )";
    }
}

std::string TermedExpressionValue::to_string() const {
    std::stringstream output{"expression "};
    output << this;
    return output.str();
}

Expression* Expression::string_literal(AST_Store& store, const std::string& value, 
    SourceLocation location) {
    
    return store.allocate_expression({ExpressionType::string_literal, value, &String, location});
}

Expression* Expression::numeric_literal(AST_Store& store, const std::string& value, 
    SourceLocation location) {
    
    return store.allocate_expression(
        {ExpressionType::numeric_literal, value, &NumberLiteral, location});
}

Expression* Expression::identifier(CompilationState& state, const std::string& value, 
    SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression(
        {ExpressionType::identifier, value, &Hole, location});
    state.unresolved_identifiers_.push_back(expression);
    return expression;
}

Expression* Expression::type_identifier(CompilationState& state, 
    const std::string& value, SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression(
        {ExpressionType::type_identifier, value, &Hole, location});
    state.unresolved_identifiers_.push_back(expression);
    return expression;
}

Expression* Expression::operator_identifier(CompilationState& state, const std::string& value, 
    SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression(
        {ExpressionType::operator_identifier, value, &Hole, location});
    state.unresolved_identifiers_.push_back(expression);
    return expression;
}

Expression* Expression::type_operator_identifier(
    CompilationState& state, const std::string& value, SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression({
        ExpressionType::type_operator_identifier, value, &Void, location});
    state.unresolved_type_identifiers_.push_back(expression);
    return expression;
}

Expression* Expression::termed(AST_Store& store, std::vector<Expression*>&& terms, 
    SourceLocation location) {
    
    return store.allocate_expression({ExpressionType::termed_expression, 
        TermedExpressionValue{terms}, &Hole, location});
}

Expression* Expression::reference(AST_Store& store, Callable* callable, 
    SourceLocation location) {
    
    return store.allocate_expression(
        {ExpressionType::reference, callable, callable->get_type(), location});
}

std::optional<Expression*> Expression::reference(AST_Store& store, const Scope& scope, 
    const std::string& name, SourceLocation location) {
    
    if (auto callable = scope.get_identifier(name))
        return reference(store, *callable, location);

    return std::nullopt;
}

Expression* Expression::type_reference(AST_Store& store, const Type* type, 
    SourceLocation location) {
    
    return store.allocate_expression({ExpressionType::type_reference, type, &Void, location});
}

Expression Expression::operator_reference(Callable* callable, SourceLocation location) {
    assert(callable->is_operator() && "AST::create_operator_ref called with not an operator");

    ExpressionType expression_type;
    
    switch (dynamic_cast<Operator*>(callable)->fixity()) {
        case Operator::Fixity::unary_prefix:
            expression_type = ExpressionType::prefix_operator_reference;
            break;
        case Operator::Fixity::unary_postfix:
            expression_type = ExpressionType::postfix_operator_reference;
            break;
        case Operator::Fixity::binary:
            expression_type = ExpressionType::binary_operator_reference;
            break;
    }

    return {expression_type, callable, callable->get_type(), location};
}

Expression* Expression::operator_reference(AST_Store& store, Callable* callable, 
    SourceLocation location) {
    
    return store.allocate_expression(operator_reference(callable, location));
}

void Expression::convert_to_operator_reference(Callable* callable) {
    auto declared_type = this->declared_type;
    *this = operator_reference(callable, location);
    this->declared_type = declared_type;
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


// valueless expression types are tie, empty, syntax_error and not_implemented
Expression* Expression::valueless(AST_Store& store, ExpressionType expression_type, 
    SourceLocation location) {
    
    return store.allocate_expression({expression_type, std::monostate{}, &Absurd, location});
}

Expression* Expression::missing_argument(AST_Store& store, const Type* type, 
    SourceLocation location) {
    
    return store.allocate_expression({ExpressionType::missing_arg, std::monostate{}, type, 
        location});
}

optional<Expression*> Expression::call(CompilationState& state, 
    Callable* callable, std::vector<Expression*>&& args, SourceLocation location) {

    auto& store = *state.ast_store_;
    auto callee_type = callable->get_type();
    
    if (!callee_type->is_function() && args.size() > 0) {
        Log::error(callable->to_string() + 
            " cannot take arguments, tried giving " + to_string(args.size()), 
            callable->location());
        return nullopt;
    }

    // TODO: move this to like typecheck file

    if (!callee_type->is_function())
        return store.allocate_expression(
            {ExpressionType::call, CallExpressionValue{callable, args}, callee_type, location});

    auto callee_f_type = dynamic_cast<const FunctionType*>(callee_type);
    auto param_types = callee_f_type->param_types();
    auto return_type = callee_f_type->return_type();

    if (args.size() > param_types.size()) {
        Log::error(callable->to_string() + " takes a maximum of " + 
            to_string(param_types.size()) + " arguments, tried giving " + to_string(args.size()), 
            location);
        return nullopt;
    }

    bool missing_args = false;
    for (auto arg: args) {
        if (arg->expression_type == ExpressionType::missing_arg)
            missing_args = true;
    }

    if (args.size() == param_types.size() && !missing_args)
        return store.allocate_expression(
            {ExpressionType::call, CallExpressionValue{callable, args}, return_type, location});

    // TODO: deal with declared types

    std::vector<const Type*> missing_arg_types{};

    for (size_t i = args.size(); i < param_types.size(); i++) {
        auto param_type = *callee_f_type->param_type(i);
        missing_arg_types.push_back(param_type);
        args.push_back(Expression::missing_argument(store, param_type, location));
    }

    auto partial_return_type = state.types_->get_function_type(
        *return_type, missing_arg_types, callee_f_type->is_pure());

    assert(args.size() == param_types.size() && 
        "Something went wrong while creating placeholders for missing args");

    return store.allocate_expression(
        {ExpressionType::partial_call, CallExpressionValue{callable, args}, 
        partial_return_type, location});
}

optional<Expression*> Expression::partial_binop_call(CompilationState& state, 
    Callable* callable, Expression* lhs, Expression* rhs, SourceLocation location) {

    auto& store = *state.ast_store_;
    auto callee_type = callable->get_type();
    
    assert(callable->is_operator() && 
        "Expression::partial_binop_call called with not an operator");
    
    assert(dynamic_cast<Operator*>(callable)->is_binary() && 
        "Expression::partial_binop_call called with not a binary operator");

    auto callee_f_type = dynamic_cast<const FunctionType*>(callee_type);
    auto return_type = callee_f_type->return_type();

    // TODO: deal with declared types

    if (lhs->expression_type == ExpressionType::missing_arg) {
        assert(callee_f_type->param_type(0) == lhs->type && 
            "Expression::partial_binop_call called with a non matching lhs type");
        
        auto partial_return_type = state.types_->get_function_type(
            *return_type, {lhs->type}, callee_f_type->is_pure());

        return store.allocate_expression(
            {ExpressionType::partial_binop_call_left, 
                CallExpressionValue{callable, {lhs, rhs}}, partial_return_type, 
                location});
    }
    
    assert(rhs->expression_type == ExpressionType::missing_arg && 
        "Expression::partial_binop_call called without a missing argument");
    
    assert(callee_f_type->param_type(1) == rhs->type && 
            "Expression::partial_binop_call called with a non matching lhs type");

    auto partial_return_type = state.types_->get_function_type(
        *return_type, {rhs->type}, callee_f_type->is_pure());
    return store.allocate_expression(
            {ExpressionType::partial_binop_call_left, 
                CallExpressionValue{callable, {lhs, rhs}}, partial_return_type, 
                location});
}

Expression* Expression::partially_applied_minus(AST_Store& store, Expression* rhs, 
    SourceLocation location) {

    return store.allocate_expression({ExpressionType::partially_applied_minus, rhs, location});
}


Expression* Expression::minus_sign(AST_Store& store, SourceLocation location) {
    return store.allocate_expression(
        Expression{ExpressionType::minus_sign, std::monostate{}, location});
}

Expression* Expression::syntax_error(AST_Store& store, SourceLocation location) {
    return store.allocate_expression(Expression{ExpressionType::syntax_error, location});
}

Operator::Precedence get_operator_precedence(const Expression& operator_ref) {
    assert(operator_ref.expression_type == ExpressionType::binary_operator_reference && 
        "get_operator_precedence called with not a binary operator reference");

    return dynamic_cast<Operator*>(operator_ref.operator_reference_value())->precedence();
}

} // namespace Maps