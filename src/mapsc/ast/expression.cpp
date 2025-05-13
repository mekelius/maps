#include "expression.hh"

#include <cassert>
#include <sstream>

#include "mapsc/logging.hh"
#include "mapsc/procedures/reverse_parse.hh"
#include "mapsc/compilation_state.hh"

using std::optional, std::nullopt, std::to_string;
using Maps::GlobalLogger::log_error;

namespace Maps {

// ----- EXPRESSION -----

DeferredBool Expression::has_native_representation() {    
    return type->is_castable_to_native();
}

std::vector<Expression*>& Expression::terms() {
    return std::get<TermedExpressionValue>(value).terms;
}
CallExpressionValue& Expression::call_value() {
    return std::get<CallExpressionValue>(value);
}
Callable* Expression::reference_value() const {
    return std::get<Callable*>(value);
}
Operator* Expression::operator_reference_value() const {
    return dynamic_cast<Operator*>(std::get<Callable*>(value));
}

bool Expression::is_partial_call() const {
    if (expression_type != ExpressionType::call)
        return false;

    auto [callee, args] = std::get<CallExpressionValue>(value);

    if (args.size() < callee->get_type()->arity())
        return true;

    for (auto arg: args) {
        if (arg->expression_type == ExpressionType::missing_arg)
            return true;
    }

    return false;
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
        return std::string{std::get<Callable*>(value)->name_};
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
        case ExpressionType::operator_reference:
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
    return (!is_identifier() && !is_illegal());
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
            return "reference to " + std::string{reference_value()->name_};
        case ExpressionType::type_reference:
            return "reference to type " + std::string{reference_value()->name_};
        case ExpressionType::operator_reference:
            return "operator " + std::string{reference_value()->name_};
        case ExpressionType::type_operator_reference:
            return "type operator " + std::string{reference_value()->name_};
        case ExpressionType::type_constructor_reference:
            return "reference to type constructor " + std::string{reference_value()->name_};
   
        case ExpressionType::type_field_name:
            return "named field" + string_value();

        case ExpressionType::syntax_error:
            return "broken expression";
        
        case ExpressionType::not_implemented:
            return "nonimplemented expression";

        case ExpressionType::call: {
            std::stringstream output{"call expression "};
            output << this;
            return output.str();
        }

        case ExpressionType::missing_arg:
            return "incomplete partial application missing argument of type " + type->to_string();

        case ExpressionType::deleted:
            return "deleted expession";

        default:
            assert(false && "unknown expression type");
    }
}

std::string TermedExpressionValue::to_string() const {
    std::stringstream output{"expression "};
    output << this;
    return output.str();
}

Expression* create_string_literal(AST_Store& store, const std::string& value, SourceLocation location) {
    return store.allocate_expression({ExpressionType::string_literal, location, value, &String});
}

Expression* create_numeric_literal(AST_Store& store, const std::string& value, SourceLocation location) {
    return store.allocate_expression({ExpressionType::numeric_literal, location, value, &NumberLiteral});
}

Expression* create_identifier_expression(CompilationState& state, const std::string& value, 
    SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression(
        {ExpressionType::identifier, location, value, &Hole});
    state.unresolved_identifiers_.push_back(expression);
    return expression;
}

Expression* create_type_identifier_expression(CompilationState& state, 
    const std::string& value, SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression(
        {ExpressionType::type_identifier, location, value, &Hole});
    state.unresolved_identifiers_.push_back(expression);
    return expression;
}

Expression* create_operator_identifier_expression(CompilationState& state, const std::string& value, 
    SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression({ExpressionType::operator_identifier, 
        location, value, &Hole});
    state.unresolved_identifiers_.push_back(expression);
    return expression;
}

Expression* create_type_operator_identifier_expression(CompilationState& state, const std::string& value, 
    SourceLocation location) {
    
    Expression* expression = state.ast_store_->allocate_expression({ExpressionType::type_operator_identifier, 
        location, value, &Void});
    state.unresolved_type_identifiers_.push_back(expression);
    return expression;
}

Expression* create_termed_expression(AST_Store& store, std::vector<Expression*>&& terms, 
    SourceLocation location) {
    
    return store.allocate_expression({ExpressionType::termed_expression, location, 
        TermedExpressionValue{terms}, &Hole});
}

Expression* create_reference_expression(AST_Store& store, Callable* callable, SourceLocation location) {
    
    return store.allocate_expression({ExpressionType::reference, location, callable, callable->get_type()});
}

Expression* create_type_reference(AST_Store& store, const Type* type, SourceLocation location) {
    return store.allocate_expression({ExpressionType::type_reference, location, type, &Void});
}

Expression* create_operator_ref(AST_Store& store, Callable* callable, SourceLocation location) {
    assert(callable->is_operator() && "AST::create_operator_ref called with not an operator");

    return store.allocate_expression(
        {ExpressionType::operator_reference, location, callable, callable->get_type()});
}

// valueless expression types are tie, empty, syntax_error and not_implemented
Expression* create_valueless_expression(AST_Store& store, ExpressionType expression_type, 
    SourceLocation location) {
    
    return store.allocate_expression({expression_type, location, std::monostate{}, &Absurd});
}

Expression* create_missing_argument(AST_Store& store, SourceLocation location, const Type* type) {
    return store.allocate_expression({ExpressionType::missing_arg, location, std::monostate{}, type});
}

optional<Expression*> create_call_expression(AST_Store& store, SourceLocation location, Callable* callable, 
    const std::vector<Expression*>& args) {

    auto callee_type = callable->get_type();
    
    if (!callee_type->is_function() && args.size() > 0) {
        log_error(std::string{callable->name_} + " cannot take arguments, tried giving " + to_string(args.size()));
        return nullopt;
    }

    // TODO: move this to like typecheck file

    if (!callee_type->is_function())
        return store.allocate_expression(
            {ExpressionType::call, location, CallExpressionValue{callable, args}, callee_type});

    auto callee_f_type = dynamic_cast<const FunctionType*>(callee_type);
    auto return_type = callee_f_type->return_type_;
    auto param_types = callee_f_type->get_params();

    if (args.size() == param_types.size())
        return store.allocate_expression(
            {ExpressionType::call, location, CallExpressionValue{callable, args}, return_type});

    if (args.size() > param_types.size()) {
        log_error(std::string{callable->name_} + " takes a maximum of " + to_string(param_types.size()) + 
            " arguments, tried giving " + to_string(args.size()));
        return nullopt;
    }
    // TODO: deal with partial calls
    // TODO: deal with declared types
    assert(false && "parial calls and all that not implemented");

    // for (int i = 0; auto arg: args) {
    //     auto param_type = param_types.at(i);

    //     if(arg->type )
    // }
}

Precedence get_operator_precedence(const Expression& operator_ref) {
    assert(operator_ref.expression_type == ExpressionType::operator_reference && 
        "get_operator_precedence called with not an operator reference");

    assert(operator_ref.reference_value()->is_binary_operator() && 
        "get_operator_precedence called with not a binary operator");

    return operator_ref.operator_reference_value()->get_precedence();
}

} // namespace Maps