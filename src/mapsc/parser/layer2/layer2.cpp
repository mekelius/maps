#include "implementation.hh"

#include <cassert>
#include <compare>
#include <memory>
#include <span>
#include <variant>

#include "mapsc/source_location.hh"
#include "mapsc/logging.hh"

#include "mapsc/compilation_state.hh"

#include "mapsc/ast/expression_properties.hh"
#include "mapsc/ast/reference.hh"
#include "mapsc/types/type.hh"
#include "mapsc/types/function_type.hh"

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/misc_expression.hh"
#include "mapsc/ast/call_expression.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/source_location.hh"


using std::optional, std::nullopt;

namespace Maps {

using Log = LogInContext<LogContext::layer2>;

// Expression types that are not allowed here
// NOTE: Empty is allowed at top-level
#define NOT_ALLOWED_IN_LAYER2 ExpressionType::identifier:\
                         case ExpressionType::type_identifier:\
                         case ExpressionType::operator_identifier:\
                         case ExpressionType::type_operator_identifier:\
                         case ExpressionType::type_operator_reference:\
                         case ExpressionType::deleted:\
                         case ExpressionType::missing_arg:\
                         case ExpressionType::user_error:\
                         case ExpressionType::compiler_error

#define TYPE_DECLARATION_TERM ExpressionType::type_argument:\
                         case ExpressionType::type_field_name:\
                         case ExpressionType::type_constructor_reference:\
                         case ExpressionType::type_reference:\
                         case ExpressionType::type_construct

// Expression types guaranteed to be simple values
#define GUARANTEED_VALUE ExpressionType::known_value

#define POTENTIAL_FUNCTION ExpressionType::call:\
                      case ExpressionType::reference:\
                      case ExpressionType::layer2_expression:\
                      case ExpressionType::ternary_expression

#define GUARANTEED_NON_OPERATOR_FUNCTION ExpressionType::lambda:\
                       case ExpressionType::partial_call


TermedExpressionParser::TermedExpressionParser(
    CompilationState* compilation_state, Expression* expression)
:expression_(expression), 
 expression_context_(expression->termed_context()),
 compilation_state_(compilation_state), 
 ast_store_(compilation_state->ast_store_.get()) {
    expression_terms_ = &expression->terms();
    next_term_it_ = expression_terms_->begin();    
}

bool TermedExpressionParser::run() {
    // some expressions might be parsed early as sub-expressions
    if (expression_->expression_type != ExpressionType::layer2_expression) {
        Log::debug_extra(expression_->location) << 
            "TermedExpressionParser::run called on a non-termed expression, skipping" << Endl;
        return true;
    }

    auto result = parse_termed_expression();

    if (!result || !success_) {
        Log::error(expression_->location) << "Parsing termed expression failed" << Endl;
        return false;
    }

    // overwrite the expression in-place
    *expression_ = **result;

    Log::debug_extra((*result)->location) << "Parsed a termed expression" << Endl;
    assert(*expression_->type != UnknownPending && "Layer2 left the type as UnknownPending");
    return true;
}

Expression* TermedExpressionParser::get_term() {
    Expression* current_term = *next_term_it_;
    assert(is_ok_in_layer2(*current_term) && 
        "TermedExpressionParser encountered a term not ok in layer2");
    next_term_it_++;
    return current_term;
}

Expression* TermedExpressionParser::peek() const {
    assert(next_term_it_ != expression_terms_->end() && "peek called with no terms left");
    return *next_term_it_;
}

Expression* TermedExpressionParser::current_term() const {
    assert(!parse_stack_.empty() && "tried to read current_term from an empty parse stack");
    return parse_stack_.back();
}

Operator::Precedence TermedExpressionParser::peek_precedence() const {
    assert((peek()->expression_type == ExpressionType::binary_operator_reference ||
            peek()->expression_type == ExpressionType::partial_binop_call_left   ||
            peek()->expression_type == ExpressionType::partial_binop_call_both   ||
            peek()->expression_type == ExpressionType::partially_applied_minus ) && 
    "TermedExpressionParser::peek_precedence called with not a binary operator on top of the stack");

    return get_operator_precedence(*peek());
}

void TermedExpressionParser::shift() {
    parse_stack_.push_back(get_term());
    Log::debug_extra(current_term()->location) << "Shift in term " << *current_term() << Endl;
}

std::optional<Expression*> TermedExpressionParser::pop_term() {
    if (parse_stack_.size() == 0)
        return std::nullopt;

    Expression* expression = parse_stack_.back();
    parse_stack_.pop_back();
    return expression;
}

void TermedExpressionParser::fail() {
    parse_stack_ = {create_user_error(*ast_store_, NO_SOURCE_LOCATION)};
    next_term_it_ = expression_terms_->end();
    expression_->type = &ErrorType;
    success_ = false;
}

bool TermedExpressionParser::at_expression_end() const {
    return next_term_it_ >= expression_terms_->end();
}

bool TermedExpressionParser::parse_stack_reduced() const {
    // quick and dirty
    // TODO: some better check
    return parse_stack_.size() == 1;
}

optional<Expression*> TermedExpressionParser::parse_termed_expression() {
    if (*expression_->type != UnknownLayer2) {
        Log::compiler_error(expression_->location) << 
            "parse_termed_expression called on an expression with type other than UnknownLayer2 ("
            << *expression_->type << ')' << Endl;

        if (*expression_->type == UnknownPending || *expression_->type == UnknownDeferred) {
            Log::error(expression_->location) << 
                "Loop detected in type dependencies, these are not handled yet. " <<
                "Try adding type annotations" << Endl;
            
            return nullopt;
        }
    }

    expression_->type = &UnknownPending;

    if (at_expression_end()) {
        Log::compiler_error(expression_->location) << "Layer2 tried to parse an empty expression" << Endl;
        fail();
        return create_user_error(*ast_store_, expression_->location);
    }


    // enter initial goto to find the first state
    shift();
    initial_goto();

    if (!success_) {
        Log::error(expression_->location) << "Parsing termed expression failed" << Endl;
        return create_user_error(*ast_store_, expression_->location);
    }

    if (!at_expression_end()) {
        Log::error(expression_->location) << 
            "Parse_termed_expression didn't parse the whole expression";
        assert(false && "parse_termed_expression didn't parse the whole expression");
        fail();
        return expression_;
    }
    
    if (!parse_stack_reduced()) {
        Log::error(expression_->location) << "Parse_termed_expression failed to reduce the stack" << Endl;
        assert(false && "parse_termed_expression failed to reduce completely");
        fail();
        return expression_;
    }

    return parse_stack_.back();
}

Expression* TermedExpressionParser::handle_termed_sub_expression(Expression* expression) {
    Log::debug_extra(current_term()->location) << "Handle termed subexpression" << Endl;

    assert(expression->expression_type == ExpressionType::layer2_expression 
        && "handle_sub_termed_expression called with non-termed expression");

    TermedExpressionParser{compilation_state_, expression}.run();

    return expression;
}


// ----- STATE FUNCTIONS -----

void TermedExpressionParser::initial_reference_state() {
    if (at_expression_end())
        return;

    reference_state();
    return initial_goto();
}

void TermedExpressionParser::initial_call_state() {
    if (at_expression_end())
        return;

    call_state();

    return initial_goto();
}

void TermedExpressionParser::initial_partial_call_state() {
    if (at_expression_end())
        return;

    assert(false && "initial partial call state not implemented");
}

// current_term is a value, so next term has to be something else, or eventually reduce to a binop
void TermedExpressionParser::initial_value_state() {
    if (at_expression_end())
        return;

    value_state();
    return initial_goto();
}

void TermedExpressionParser::reduce_to_partial_binop_call_left() {
    Expression* rhs = *pop_term();
    Expression* op = *pop_term();

    // TODO: handle precedence here
    Expression* missing_argument = create_missing_argument(*ast_store_,
        *dynamic_cast<const FunctionType*>(op->type)->param_types().begin(), op->location);

    auto call =
        create_partial_binop_call(*compilation_state_,
            op->operator_reference_value(), missing_argument, rhs, expression_->location);
    
    if (!call) {
        Log::error(op->location) << "Creating call expression failed" << Endl;
        return fail();
    }

    parse_stack_.push_back(*call);
}

void TermedExpressionParser::reduce_to_partial_binop_call_right() {
    Expression* op = *pop_term();
    Expression* lhs = *pop_term();

    // TODO: handle precedence here
    Expression* missing_argument = create_missing_argument(*ast_store_,
        *dynamic_cast<const FunctionType*>(op->type)->param_types().begin(), op->location);

    auto call =
        create_partial_binop_call(*compilation_state_,
            op->operator_reference_value(), lhs, missing_argument, expression_->location);
    
    if (!call) {
        Log::error(op->location) << "Creating call expression failed" << Endl;
        return fail();
    }

    parse_stack_.push_back(*call);
}

void TermedExpressionParser::reduce_to_partial_binop_call_both() {
    assert(false && "not implemented");
}


void TermedExpressionParser::reduce_to_unary_minus_ref() {
    parse_stack_.push_back(unary_minus_ref((*pop_term())->location));
}

void TermedExpressionParser::reduce_prefix_operator() {
    auto value = *pop_term();
    auto op = *pop_term();
    
    assert(op->expression_type == ExpressionType::prefix_operator_reference && 
        "reduce_prefix_operator called with not a prefix operator 2nd on the stack");

    auto expression = create_call(*compilation_state_, op->reference_value(), 
        {value}, op->location);

    if (!expression) {
        Log::error(op->location) << "Applying unary prefix " << *op << " to " << 
            *value << " failed" << Endl;
        return fail();
    }

    parse_stack_.push_back(*expression);
}

void TermedExpressionParser::reduce_postfix_operator() {
    auto op = *pop_term();
    auto value = *pop_term();

    assert(op->expression_type == ExpressionType::postfix_operator_reference && 
        "reduce_postfix_operator called with not a postfix operator 1nd on the stack");

    auto expression = create_call(*compilation_state_, op->reference_value(), 
        {value}, op->location);

    if (!expression) {
        Log::error(op->location) << "Applying unary postfix " << *op << " to " << 
            *value << " failed" << Endl;
        return fail();
    }

    parse_stack_.push_back(*expression);
}

void TermedExpressionParser::reduce_partially_applied_minus() {
    auto arg = *pop_term();

    assert(current_term()->expression_type == ExpressionType::minus_sign && 
        "reduce_partially_applied_minus called with not a minus sign 2nd on the stack");
    auto location = (*pop_term())->location;

    if (arg->expression_type == ExpressionType::partially_applied_minus)
        if (!convert_to_unary_minus_call(*compilation_state_, *arg)) {
            Log::error(current_term()->location) << "Creating umary minus call failed" << Endl;
            return fail();
        }

    auto expression = create_partially_applied_minus(
        *ast_store_, arg, location);
    return parse_stack_.push_back(expression);
}

void TermedExpressionParser::initial_goto() {
    switch (current_term()->expression_type) {
        case ExpressionType::layer2_expression:
            handle_termed_sub_expression(current_term());
            return initial_goto();

        // case ExpressionType::lambda:
        //     assert(false && "not implemented");
        //     // initial_definition_state();
        //     break;

        case ExpressionType::partial_binop_call_both:
            assert(false && "not implemented");

        case ExpressionType::ternary_expression:
        case ExpressionType::reference:
            initial_reference_state();
            break;

        case ExpressionType::partial_binop_call_left:
            initial_partial_binop_call_left_state();
            break;

        case ExpressionType::partial_binop_call_right:
            initial_partial_binop_call_right_state();
            break;

        case ExpressionType::partial_call:
            initial_partial_call_state();
            break;

        case ExpressionType::call:
        case GUARANTEED_VALUE:
            initial_value_state();
            break;

        case ExpressionType::known_value_reference:
            known_value_reference_state();
            return initial_goto();

        case ExpressionType::binary_operator_reference:
            initial_binary_operator_state();
            break;

        case ExpressionType::prefix_operator_reference:
            initial_prefix_operator_state();
            break;

        case ExpressionType::postfix_operator_reference:
            initial_postfix_operator_state();
            break;

        case ExpressionType::partially_applied_minus:
            initial_partially_applied_minus_state();
            break;

        case ExpressionType::minus_sign:
            initial_minus_sign_state();
            break;

        case ExpressionType::type_reference:
            initial_type_reference_state();
            break;

        case ExpressionType::type_field_name:
        case ExpressionType::type_argument:
        case ExpressionType::type_construct:
        case ExpressionType::type_constructor_reference:
        case NOT_ALLOWED_IN_LAYER2:
            Log::error(expression_->location) << 
                "bad term type: " << current_term()->expression_type_string() << Endl;
            return fail();
    }
}

void TermedExpressionParser::initial_binary_operator_state() {
    binary_operator_state();
    initial_goto();
}

void TermedExpressionParser::initial_prefix_operator_state() {
    if (at_expression_end()) {
        auto op = *pop_term();
        return parse_stack_.push_back(create_reference(
            *ast_store_, op->operator_reference_value(), op->location));
    }

    prefix_operator_state();
    return initial_goto();
}

void TermedExpressionParser::initial_postfix_operator_state() {
    if (at_expression_end()) {
        auto op = *pop_term();
        return parse_stack_.push_back(create_reference(
            *ast_store_, op->operator_reference_value(), op->location));
    }

    Log::error(current_term()->location) << "Unexpected postfix operator " << *current_term() << 
        "at the start of an expression";
    return fail();
}

void TermedExpressionParser::initial_minus_sign_state() {
    minus_sign_state();
    return initial_goto();
}

void TermedExpressionParser::initial_partially_applied_minus_state() {
    if (at_expression_end())
        return;

    if (!convert_to_unary_minus_call(*compilation_state_, *current_term())) {
        Log::error(current_term()->location) << "Converting to unary minus failed" << Endl;
        return fail();
    }

    return initial_call_state();
}

// Basically we just unwrap it
void TermedExpressionParser::initial_partial_binop_call_right_state() {
    partial_binop_call_right_state();

    if (!at_expression_end())
        return initial_goto();
}

void TermedExpressionParser::initial_partial_binop_call_left_state() {
    if (at_expression_end())
        return;

    assert(false && "not implemented");
}

void TermedExpressionParser::initial_partial_binop_call_both_state() {
    if (at_expression_end())
        return;

    assert(false && "not implemented");
}

void TermedExpressionParser::initial_type_reference_state() {
    assert(current_term()->expression_type == ExpressionType::type_reference && 
        "TermedExpressionParser::type_specifier_state entered with not a type_specifier/type_reference \
on the stack");

    type_reference_state();
    initial_goto();
}

void TermedExpressionParser::reference_state() {
    if (at_expression_end())
        return;

    // if it's not a function it's treated as a value
    // i.e. the next term has to be an operator
    if (current_term()->type->arity() == 0) {
        convert_nullary_reference_to_call(*current_term());
        return value_state();
    }

    switch (peek()->expression_type) {
        case ExpressionType::layer2_expression:
            handle_termed_sub_expression(peek());
            return reference_state();

        case GUARANTEED_VALUE:
        case ExpressionType::reference:
        case ExpressionType::call:
            return call_expression_state();

        case ExpressionType::binary_operator_reference:
            precedence_stack_.push_back(peek_precedence());
            shift();
            return post_binary_operator_state();

        default:
            assert(false && "unhandled term type in TermedExpressionParser::initial_identifier_state");
    }
}

void TermedExpressionParser::value_state() {
    Log::debug_extra(current_term()->location) << "Value state" << Endl;

    if (at_expression_end())
        return;

    if (current_term()->type->is_function())
        return call_state();

    switch (peek()->expression_type) {
        // case ExpressionType::lambda:
        case ExpressionType::ternary_expression:
            assert(false && "not implemented");

        case ExpressionType::layer2_expression:
            handle_termed_sub_expression(peek());
            return value_state();

        case ExpressionType::partial_binop_call_both:
            assert(false && "not implemented");

        case ExpressionType::binary_operator_reference: {
            shift();
            auto op = current_term()->operator_reference_value();
            
            precedence_stack_.push_back(dynamic_cast<const Operator*>(op)->precedence());
            return post_binary_operator_state();
        }
        case ExpressionType::prefix_operator_reference:
            shift();
            prefix_operator_state();
            switch (current_term()->expression_type) {
                default:
                    assert(false && "goto table not implemented");
            }

        case ExpressionType::postfix_operator_reference:
            push_unary_operator_call(get_term(), *pop_term());
            return call_state();

        case ExpressionType::minus_sign: {
            auto location = get_term()->location;
            parse_stack_.push_back(binary_minus_ref(location));
            precedence_stack_.push_back(
                dynamic_cast<const Operator*>(current_term()->operator_reference_value())->precedence());
            return post_binary_operator_state();
        }

        case ExpressionType::known_value_reference:
            substitute_known_value_reference(peek());
            // intentional fall-through
        case ExpressionType::reference:
        case ExpressionType::call:
        case GUARANTEED_VALUE:
            Log::error(peek()->location) << "unexpected " << *peek() <<
                " in termed expression, expected an operator" << Endl;
            return fail();

        case ExpressionType::partially_applied_minus:
            if (!convert_to_partial_binop_call_left(*compilation_state_, *peek())) {
                Log::error(current_term()->location) << "Converting to partial binop minus failed" << Endl;
                return fail();
            }

            // intentional fall-through
        case ExpressionType::partial_binop_call_left: {
            auto location = current_term()->location;
            add_to_partial_call_and_push(get_term(), {*pop_term()}, location);
            return call_state();
        }
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_call:
            Log::error(peek()->location) << "Unexpected partial call, expected an operator" << Endl;
            return fail();

        case ExpressionType::type_reference:
        case ExpressionType::type_argument:
        case ExpressionType::type_construct:
        case ExpressionType::type_constructor_reference:
            assert(false && "not implemented");
    
        case ExpressionType::type_field_name:
        case NOT_ALLOWED_IN_LAYER2:
            // TODO: make expression to_str
            Log::error(peek()->location) << "bad term "<< *peek() << " in initial value state" << Endl;
            return fail();
    }
}

void TermedExpressionParser::known_value_reference_state() {
    substitute_known_value_reference(current_term());
    return value_state();
}

void TermedExpressionParser::prefix_operator_state() {
    Log::debug_extra(current_term()->location) << "Prefix operator state" << Endl;

    if (at_expression_end()) {
        auto op = *pop_term();
        return parse_stack_.push_back(create_reference(
            *ast_store_, op->operator_reference_value(), op->location));
    }

    switch (peek()->expression_type) {
        case ExpressionType::layer2_expression:
            handle_termed_sub_expression(peek());
            return prefix_operator_state();

        case GUARANTEED_VALUE:
            return push_unary_operator_call(*pop_term(), get_term());

        case ExpressionType::prefix_operator_reference:
            shift();
            prefix_operator_state();
            return reduce_prefix_operator();

        case ExpressionType::postfix_operator_reference:
        case NOT_ALLOWED_IN_LAYER2:
            Log::compiler_error(peek()->location) << 
                "Unexpected " << *peek() << " in termed expression" << Endl;
            return fail();
            
        case ExpressionType::reference:
        case ExpressionType::call:
            shift();
            reduce_prefix_operator();

            return;

        default:
            Log::compiler_error(current_term()->location) << 
                "Unexpected " << *current_term() << " after an unary operator" << Endl;
            return fail();
    }
    Log::error(current_term()->location) << 
        "Unexpected " << *current_term() << " after an unary operator" << Endl;
    return fail();
}

void TermedExpressionParser::binary_operator_state() {
    if (at_expression_end())
        return;        

    switch (peek()->expression_type) {
        case ExpressionType::layer2_expression:
            handle_termed_sub_expression(peek());
            return binary_operator_state();

        // case ExpressionType::lambda:
        case ExpressionType::ternary_expression:
        case ExpressionType::partial_binop_call_both:
            assert(false && "not implemented");

        case ExpressionType::known_value_reference:
            substitute_known_value_reference(peek());
            // intentional fall-through
        case GUARANTEED_VALUE:
            precedence_stack_.push_back(
                dynamic_cast<const Operator*>(current_term()->operator_reference_value())
                    ->precedence());
            shift();
            value_state();

            // TODO: look at the precedence stack

            switch (current_term()->expression_type) {
                case GUARANTEED_VALUE:
                case POTENTIAL_FUNCTION:
                    reduce_to_partial_binop_call_left();
                    return partial_binop_call_left_state();

                case ExpressionType::partial_binop_call_right:
                    reduce_to_partial_binop_call_both();
                    return partial_binop_call_both_state();

                default:
                    Log::error(current_term()->location) << 
                        "Unexpected " << *current_term() << ", expected a value";
                    return fail();
            }

        case ExpressionType::minus_sign:{
            auto location = get_term()->location;
            switch (peek()->expression_type) {
                case GUARANTEED_VALUE:
                    push_unary_operator_call(unary_minus_ref(location), get_term());
                    assert(false && "not implemented");

                default:
                    assert(false && "not implemented");
            }
            assert(false && "not implemented");
        }

        case ExpressionType::partially_applied_minus:
        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_call:
        case ExpressionType::binary_operator_reference:
            assert(false && "not implemented");

        case ExpressionType::prefix_operator_reference:{
            shift();
            prefix_operator_state();
            auto location = current_term()->location;
            if (auto value = pop_term()) {
                auto missing_arg_type = 
                    dynamic_cast<const FunctionType*>(
                        current_term()->operator_reference_value()->get_type())
                            ->param_type(1);
                return push_partial_call(*pop_term(), 
                    {create_missing_argument(*ast_store_, *missing_arg_type, location), *value});
            }
            Log::error(location) << "unary operator failed to produce a value" << Endl;
            return fail();
        }

        case ExpressionType::postfix_operator_reference:
            Log::error(peek()->location) << "Postfix unary operator " << *peek() << 
                " not allowed to operate on " << *current_term() << Endl;
            return fail(); 

        case ExpressionType::reference:
            shift();
            reference_state();
            switch (current_term()->expression_type) {
                case GUARANTEED_VALUE:
                case ExpressionType::reference:
                case ExpressionType::call:
                    return reduce_to_partial_binop_call_left();
                
                default:
                    Log::error(current_term()->location) << "Unexpected " << 
                        *current_term() << " as right side of binary operator, expected a value";
                    return fail();
            }

        case ExpressionType::call:
            shift();
            call_state();
            switch (current_term()->expression_type) {
                case GUARANTEED_VALUE:
                case ExpressionType::reference:
                case ExpressionType::call:
                    return reduce_to_partial_binop_call_left();

                default:
                    Log::error(current_term()->location) << "Unexpected " << *current_term() << 
                        " as right side of binary operator, expected a value";
                    return fail();
            }

        case TYPE_DECLARATION_TERM:
            assert(false && "not implemented");

        case NOT_ALLOWED_IN_LAYER2:
            assert(false && "bad term encountered in TermedExpressoinParser");
    }
}

void TermedExpressionParser::minus_sign_state() {
    Log::debug_extra(current_term()->location) << "Minus sign state" << Endl;

    if (at_expression_end()) {
        auto declared_type = current_term()->declared_type;

        if (!declared_type) {
            Log::error(current_term()->location) << 
                "minus sign without arguments or declared type is ambiguous";
            return fail();
        }

        if (declared_type == &Int_to_Int)
            return reduce_to_unary_minus_ref();

        if (declared_type == &IntInt_to_Int)
            return parse_stack_.push_back(binary_minus_ref(current_term()->location));

        Log::error(current_term()->location) << "Type " << **declared_type << 
            " is not allowed with \"-\"";
        return fail();
    }

    switch (peek()->expression_type) {
        case ExpressionType::layer2_expression:
            handle_termed_sub_expression(peek());
            return minus_sign_state();

        case GUARANTEED_VALUE: {
            shift();

            if (at_expression_end())
                return reduce_partially_applied_minus();

            switch (peek()->expression_type) {
                case ExpressionType::binary_operator_reference:
                    reduce_minus_sign_to_unary_minus_call();
                    shift();
                    return post_binary_operator_state();

                case ExpressionType::partial_binop_call_left: {
                    reduce_minus_sign_to_unary_minus_call();
                    auto location = current_term()->location;
                    add_to_partial_call_and_push(get_term(), {*pop_term()}, location);
                    return call_state();
                }

                case ExpressionType::minus_sign:
                    reduce_minus_sign_to_unary_minus_call();
                    parse_stack_.push_back(binary_minus_ref(get_term()->location));
                    return post_binary_operator_state();

                case TYPE_DECLARATION_TERM:
                    assert(false && "Type declaration after a minus term not implemented");

                default:
                    Log::error(peek()->location) << 
                        "Unexpected " << *peek() << " after a value, expected an operator";
                    return fail(); 
            }

            return value_state();
        }

        case ExpressionType::minus_sign:
            shift();
            minus_sign_state();
            return reduce_partially_applied_minus();

        case ExpressionType::partially_applied_minus:
            shift();
            if (!convert_to_unary_minus_call(*compilation_state_, *current_term())) {
                Log::error(current_term()->location) << "Converting to unary minus failed" << Endl;
                return fail(); 
            }
            return reduce_partially_applied_minus();

        case ExpressionType::reference:
        case ExpressionType::call:
            shift();
            reduce_partially_applied_minus();
            return partially_applied_minus_state();

        case TYPE_DECLARATION_TERM:
        default:
            Log::error(current_term()->location) << "Initial minus sign not allowed with " << *peek() << Endl;
            return fail(); 
    }
}

void TermedExpressionParser::partially_applied_minus_state() {
    if (at_expression_end())
        return;

    if(!convert_to_unary_minus_call(*compilation_state_, *current_term())) {
        Log::error(current_term()->location) << "Converting to unary minus failed" << Endl;
        return fail(); 
    }

    return call_state();
}

// Basically we just unwrap it
void TermedExpressionParser::partial_binop_call_right_state() {
    if (at_expression_end())
        return;

    auto location = current_term()->location;
    auto [callee, args] = current_term()->call_value();
    assert(callee->is_operator() 
        && "initial_partial_binop_call_right_state called with not an operator call");
    assert(dynamic_cast<const Operator*>(callee)->is_binary() && 
        "initial_partial_binop_call_right_state called with not a binary operator call");

    assert(((args.size() == 2 && args.at(1)->expression_type == ExpressionType::missing_arg) || 
            args.size()) == 1 && 
                "initial_partial_binop_call_right_state called with invalid args");

    // auto precedence = dynamic_cast<Operator*>(callee)->precedence();

    switch (peek()->expression_type) {
        case ExpressionType::binary_operator_reference:
            convert_to_partial_call(*current_term()); // treat it as a value
            shift();
            return post_binary_operator_state();

        case ExpressionType::postfix_operator_reference:
            convert_to_partial_call(*current_term());
            shift();
            return reduce_postfix_operator(); // It could be an operator that can act on this partial call

        case ExpressionType::partial_binop_call_left:
            return partial_binop_call_standoff_state();

        case GUARANTEED_VALUE:
        case ExpressionType::call:
        case ExpressionType::reference:
        case ExpressionType::partially_applied_minus:
        case ExpressionType::prefix_operator_reference:
        case ExpressionType::minus_sign:
            break;

        case TYPE_DECLARATION_TERM:
            assert(false && "type declarations not implemented here");
        case NOT_ALLOWED_IN_LAYER2:
        default:
            Log::error(peek()->location) << "Unexpected " << *peek() << ", expected a value" << Endl;
            return fail(); 
    }

    pop_term();
    parse_stack_.push_back(args.at(0));
    parse_stack_.push_back(create_operator_reference(*ast_store_, dynamic_cast<const Operator*>(callee), 
        location));
    return post_binary_operator_state();
}

void TermedExpressionParser::partial_binop_call_left_state() {
    if (at_expression_end())
        return;

    assert(false && "not implemented");
}

void TermedExpressionParser::partial_binop_call_both_state() {
    if (at_expression_end())
        return;

    assert(false && "not implemented");
}

void TermedExpressionParser::call_state() {
    if (at_expression_end())
        return;

    auto type_to_use = current_term()->declared_type ?
        *current_term()->declared_type : current_term()->type;

    if (*type_to_use == Unknown) {
        switch (peek()->expression_type) {
            default:
                assert(false && "Initial call type inference not implemented");
        }
    }

    // known non-nullary function
    if (type_to_use->arity() > 0)
        return deferred_call_state();

    // known nullary type
    return value_state();
}

void TermedExpressionParser::partial_binop_call_standoff_state() {
    assert(false && "not implemented");
}

void TermedExpressionParser::post_binary_operator_state() {
    Log::debug_extra(current_term()->location) << "Post-binary operator state" << Endl;

    if (at_expression_end())
        return reduce_to_partial_binop_call_right();

    switch (peek()->expression_type) {
        case ExpressionType::layer2_expression:
            handle_termed_sub_expression(peek());
            return post_binary_operator_state();

        case ExpressionType::binary_operator_reference:
        case ExpressionType::prefix_operator_reference:
        case ExpressionType::postfix_operator_reference:
        // case ExpressionType::lambda:
        case ExpressionType::ternary_expression:
            assert(false && "not implemented");

        case ExpressionType::reference:
            shift();
            // if the reference is to a function, go ahead and try to apply it first
            if (current_term()->reference_value()->get_type()->arity() > 0)
                call_expression_state();

            return compare_precedence_state();

        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::call: {
            shift();
            
            // if we have a complete call, we can treat it as a value
            // if the call is partial, try to complete it first
            // NOTE: partial call_state could also return a partial call, in which case
            //       we can try to apply the operator onto that
            if (is_partial_call(*current_term())) {
                partial_call_state();
            }
            
            return compare_precedence_state();
        }

        case ExpressionType::minus_sign:
            parse_stack_.push_back(unary_minus_ref(get_term()->location));
            prefix_operator_state();

            if (has_failed())
                return;

            return compare_precedence_state();

        case ExpressionType::partially_applied_minus:
            shift();

            if (!convert_to_unary_minus_call(*compilation_state_, *current_term())) {
                Log::error(current_term()->location) << "Converting to unary minus failed" << Endl;
                return fail(); 
            }

            return compare_precedence_state();

        case ExpressionType::partial_binop_call_both:
        case ExpressionType::partial_call:
            assert(false && "not implemented");
            
        case ExpressionType::known_value_reference:
            substitute_known_value_reference(peek());
            // intentional fall-through
        case GUARANTEED_VALUE:
            shift();
            return compare_precedence_state();

        case TYPE_DECLARATION_TERM:
            if (peek()->expression_type == ExpressionType::type_reference) {
                shift();
                type_reference_state();
                return compare_precedence_state();
            }

            assert(false && "not implemented");

        case NOT_ALLOWED_IN_LAYER2:
            assert(false && "bad term in TermedExpressionParser::post_binary_operator_state");
            break;
    }
}

// stack state: REDUCED_TREE OP1 VAL | input state: OP2(?)
// so, we compare the precedences of the operators
// if OP1 wins, we can reduce left, if OP2 wins we must wait (how?)
void TermedExpressionParser::compare_precedence_state() {
    Log::debug_extra(current_term()->location) << "Compare precedence state" << Endl;

    if (at_expression_end()) {
        reduce_binop_call();
        precedence_stack_.pop_back();
        return;
    }

    switch (peek()->expression_type) {
        case ExpressionType::layer2_expression:
            handle_termed_sub_expression(peek());
            return compare_precedence_state();

        case ExpressionType::partially_applied_minus:
            if (!convert_to_partial_binop_call_left(*compilation_state_, *peek())) {
                Log::error(peek()->location) << 
                    "Converting minus to partial binop call failed" << Endl;
                return fail();
            }

            // intentional fall-through
        case ExpressionType::partial_binop_call_left:
            return compare_precedence_to_partial_binop_call_left_state();

        case ExpressionType::partial_binop_call_both:
            assert(false && "not implemented");

        case ExpressionType::binary_operator_reference:
            break;

        case ExpressionType::minus_sign:
            convert_to_operator_reference(*peek(),
                compilation_state_->special_definitions_.binary_minus);
            break;

        default:
            reduce_binop_call();
            precedence_stack_.pop_back();
            return;
    }

    // precedence goes down => one or more "closing parenthesis" required
    auto next_precedence = peek_precedence();

    if (precedence_stack_.back() > next_precedence) {
        reduce_binop_call();
        precedence_stack_.pop_back();
        
        // if there's an "intermediate level" or operators, we need to handle those before returning
        if (precedence_stack_.back() < next_precedence) {
            precedence_stack_.push_back(next_precedence);
            shift();
            return post_binary_operator_state();
        }
        
        // otherwise simply return 
        return;
    }

    assert(peek()->expression_type == ExpressionType::binary_operator_reference
        && "compare_precedence_state called with not an operator ref next");

    if (precedence_stack_.back() == next_precedence) { // !!!: assuming left-associativity
        // if the precedence stays the same, just reduce and carry on
        reduce_binop_call();
        shift();
        return post_binary_operator_state();
    }

    // if the precedence goes up, push it into the stack and run post_binary_op_state with 
    // the new precedence stack
    // once it returns, there should be a nice reduced rhs on the parse_stack_
    if (precedence_stack_.back() < next_precedence) {
        unsigned int previous_precedence = precedence_stack_.back();
        precedence_stack_.push_back(next_precedence);
        shift();
        post_binary_operator_state();
        assert(precedence_stack_.back() == previous_precedence 
            && "post_binary_operator_state didn't return to the same precedence level");
            
        reduce_binop_call();

        if (at_expression_end()) {
            // ??? should we look at the precedence stack here? 
            return;
        }

        assert(peek_precedence() >= precedence_stack_.back() 
            && "post_binary_operator_state didn't run until the end or a lover/equal precedence \
that the caller put on the precedence stack");
        
        // continue parsing
        shift();
        return post_binary_operator_state(); /// how about the shifts here
    }
}

void TermedExpressionParser::compare_precedence_to_partial_binop_call_left_state() {
    Log::debug_extra(current_term()->location) << 
        "Compare precedence to partial binop call left state" << Endl;

    if (at_expression_end()) {
        assert(false && "compare_precedence_to_partial_binop... called incorrectly");
        return fail();
    }

    auto next_precedence = peek_precedence();
    auto prev_precedence = precedence_stack_.back();

    if (next_precedence < prev_precedence) {
        reduce_binop_call();
        if (has_failed())
            return;

        shift();
        precedence_stack_.pop_back();
        return reduce_partial_binop_call_left();
    }

    shift();
    reduce_partial_binop_call_left();
    precedence_stack_.pop_back();
    return reduce_binop_call();
}

// Pops 3 values from the parse stack, reduces them into a binop apply expression and pushes it on top
void TermedExpressionParser::reduce_binop_call() {
    Log::debug_extra(current_term()->location) << "Reducing binop call" << Endl;

    assert(parse_stack_.size() >= 3 
        && "TermedExpressionParser::reduce_operator_left_ called with parse stack size < 3");

    Expression* rhs = *pop_term();
    Expression* operator_ = *pop_term();
    Expression* lhs = *pop_term();

    auto function_type = 
        dynamic_cast<const FunctionType*>(operator_->operator_reference_value()->get_type());
    
    assert(function_type->arity() >= 2 && "binop with arity less than 2");

    if (!ensure_proper_argument_expression_type(lhs, *function_type->param_type(0)) || 
        !ensure_proper_argument_expression_type(rhs, *function_type->param_type(1))
    ) {
        Log::error(lhs->location) << "Invalid args for binary expression" << Endl;
        return fail(); 
    }

    // TODO: check types here
    assert(std::holds_alternative<const DefinitionHeader*>(operator_->value) && 
        "TermedExpressionParser::reduce_operator_left called with a call stack \
where operator didn't hold a reference to a definition");

    auto reduced = create_call(*compilation_state_,
        std::get<const DefinitionHeader*>(operator_->value), {lhs, rhs}, lhs->location);

    if (!reduced) {
        Log::error(rhs->location) << "Creating a binary operator call failed" << Endl;
        return fail(); 
    }

    parse_stack_.push_back(*reduced);
}

void TermedExpressionParser::reduce_partial_binop_call_left() {
    Log::debug_extra(current_term()->location) << "Reducing partial binop call left" << Endl;

    Expression* call = *pop_term();
    Expression* arg = *pop_term();

    assert(is_partial_call(*call));
    assert(is_binop_left(*call));

    if (!ensure_proper_argument_expression_type(arg, 
        *dynamic_cast<const FunctionType*>(call->type)->param_type(0))) {
        Log::error(arg->location) << "Invalid arg for binary expression" << Endl;
        return fail(); 
    }

    auto reduced = complete_partial_binop_call_left(*compilation_state_, *call, *arg);
    if (!reduced) {
        Log::error(call->location) << "Reducing partial binop call failed" << Endl;
        return fail();
    }

    parse_stack_.push_back(*reduced);
}

void TermedExpressionParser::reduce_minus_sign_to_unary_minus_call() {
    Log::debug_extra(current_term()->location) << "Reducing unary minus call" << Endl;

    auto arg = pop_term();
    assert(arg && "reduce_unary_minus_call called with nothing on the stack");

    assert(current_term()->expression_type == ExpressionType::minus_sign && 
        "reduce_unary_minus_call called with no minus sign on the stack");
    auto location = (*pop_term())->location; // eat the minus

    auto call = create_call(*compilation_state_, &unary_minus_Int, {*arg}, location);
    if (!call) {
        Log::error(location) << "Creating unary call to - failed" << Endl;
        return fail(); 
    }

    parse_stack_.push_back(*call);
}

void TermedExpressionParser::type_reference_state() {
    Log::debug_extra(current_term()->location) << "Type reference state" << Endl;

    if (at_expression_end()) {
        if (!possibly_type_expression_) {
            Log::error(current_term()->location) << "Unexpected type expression at expression end" << Endl;
            return fail();
        }
        return;
    }

    switch (peek()->expression_type) {
        case ExpressionType::layer2_expression:
            handle_termed_sub_expression(peek());
            return type_reference_state();

        case ExpressionType::call:
            apply_type_declaration_and_push(*pop_term(), get_term());
            return call_state();

        case GUARANTEED_VALUE: {
            auto type_term = *pop_term();
            assert(std::holds_alternative<const Type*>(type_term->value) && "no type");
            auto type_value = std::get<const Type*>(type_term->value);

            shift();
            auto term = current_term();

            Log::debug_extra(type_term->location) << "Applying " << *type_term << " to " << *term << Endl;
            if (!term->cast_to(*compilation_state_, type_value)) {
                Log::error(type_term->location) << 
                    "Could not apply type declaration " << *type_term << " to " << *term << Endl;
                return fail();
            }

            term->declared_type = type_value;
            term->location = type_term->location;

            ast_store_->delete_expression(type_term);

            return value_state();
        }
        case ExpressionType::reference: {
            auto type_term = *pop_term();
            auto type_value = type_term->value;
            ast_store_->delete_expression(type_term);
            
            assert(std::holds_alternative<const Type*>(type_value) && "no type");
            shift();
            current_term()->declared_type = get<const Type*>(type_value);
            current_term()->location = type_term->location;
            return reference_state(); 
        }
        case ExpressionType::binary_operator_reference: {
            shift();
            binary_operator_state();
            auto value = pop_term();
            return apply_type_declaration_and_push(*pop_term(), *value);
        }

        case ExpressionType::minus_sign: {
            shift();
            minus_sign_state();
            auto value = pop_term();
            return apply_type_declaration_and_push(*pop_term(), *value);
        }
        case ExpressionType::partially_applied_minus: {
            if (current_term()->type_reference_value()->arity() != 0)
                convert_to_partial_call(*peek());
            return apply_type_declaration_and_push(*pop_term(), get_term());
        }
        case ExpressionType::type_reference:
        default:
            Log::error(peek()->location) << 
                "Unexpected " << *peek() << " after a type reference, expected an expression";
            return fail();

        case NOT_ALLOWED_IN_LAYER2:
            Log::compiler_error(current_term()->location) << 
                "bad term encountered in TermedExpressoinParser";
            assert(false && "bad term encountered in TermedExpressoinParser");
            return fail();
    }
}

void TermedExpressionParser::push_partial_call(Expression* callee_ref, 
    const std::vector<Expression*>& args) {

    push_partial_call(callee_ref, args, callee_ref->location);
}
void TermedExpressionParser::push_partial_call(Expression* callee_ref, 
    const std::vector<Expression*>& args, SourceLocation location) {
    
    auto call = create_call(*compilation_state_, 
        callee_ref->reference_value(), std::vector<Expression*>{args}, location);
    
    if (!call) {
        Log::error(location) << "During layer2: Creating partial call failed" << Endl;
        return fail(); 
    }
        
    parse_stack_.push_back(*call);
}

void TermedExpressionParser::add_to_partial_call_and_push(Expression* partial_call, 
    const std::vector<Expression*>& args, SourceLocation location) {

    assert(is_partial_call(*partial_call) && 
        "TermedExpressionParser::add_to_partial_call_and_push called with not a partial call");

    auto [callee, old_args] = partial_call->call_value();
    std::vector<Expression*> new_args;

    auto new_arg_it = args.begin();
    for (auto arg: old_args) {
        if (arg->expression_type == ExpressionType::missing_arg && new_arg_it != args.end()) {
            new_args.push_back(*new_arg_it);
            new_arg_it++;
            continue;
        }

        new_args.push_back(arg);
    }

    auto expression = create_call(*compilation_state_, callee, std::move(new_args), 
        location);

    if (!expression) {
        Log::error(location) << "Creating partial call to " << *callee << " failed" << Endl;
        return fail(); 
    }

    return parse_stack_.push_back(*expression);
}

void TermedExpressionParser::push_unary_operator_call(Expression* operator_ref, Expression* value) {
    auto location = 
        dynamic_cast<const Operator*>(
            operator_ref->operator_reference_value())->fixity() == Operator::Fixity::unary_prefix ?
            operator_ref->location : value->location;
    auto call = create_call(*compilation_state_, 
        operator_ref->reference_value(), { value }, location);
            
    if (!call) {
        Log::error(location) << "Creating a call to unary operator failed" << Endl;
        return fail(); 
    }

    parse_stack_.push_back(*call);

    return initial_value_state();
}

void TermedExpressionParser::apply_type_declaration_and_push(Expression* type_declaration, Expression* value) {
    Log::debug_extra(type_declaration->location) << 
        "Applying type declaration " << *type_declaration << " to " << *value;

    auto new_expression = value->cast_to(*compilation_state_, type_declaration->type_reference_value());

    if (!new_expression) {
        parse_stack_.push_back(create_user_error(*ast_store_, type_declaration->location));
        Log::error(type_declaration->location) << "Applying type declaration failed" << Endl;
        return fail(); 
    }

    assert(*(*new_expression)->type == *type_declaration->type_reference_value() &&
        "Applying type declaration didn't change the type");
    parse_stack_.push_back(*new_expression);
}

void TermedExpressionParser::substitute_known_value_reference(Expression* known_value_reference) {
    assert(known_value_reference->expression_type == ExpressionType::known_value_reference &&
        "substitute_known_value_reference called with not a value reference");

    Log::debug_extra(known_value_reference->location) << "Substituting known value reference term" << Endl;

    if (!convert_by_value_substitution(*known_value_reference)) {
        Log::error(known_value_reference->location) << 
            "Value substitution of " << *known_value_reference << "failed" << Endl;
        return fail();
    }
}

bool TermedExpressionParser::is_acceptable_next_arg(const DefinitionHeader* callee, 
    const std::vector<Expression*>& args/*, Expression* next_arg*/) {
    if (args.size() >= callee->get_type()->arity())
        return false;

    // TODO: check type
    return true;
}

bool TermedExpressionParser::ensure_proper_argument_expression_type(
    Expression* arg, const Type* param_type) {
    
    if (is_allowed_as_arg(*arg))
        return true;

    if (arg->expression_type == ExpressionType::partially_applied_minus)
        return convert_partially_applied_minus_to_arg(*compilation_state_, *arg, param_type);

    return false;
}

void TermedExpressionParser::call_expression_state() {
    Expression* reference = *pop_term();
    
    assert(reference->expression_type == ExpressionType::reference 
        && "TermedExpressionParser called with a callee that was not a reference");
    assert(reference->type->arity() > 0 && "call_expression_state called with arity 0");

    std::vector<Expression*> args;
    
    // parse args
    for (unsigned int i = 0; i < reference->type->arity(); i++) {
        shift();

        args.push_back(handle_arg_state(reference->reference_value(), args));

        if (at_expression_end()) {
            // accept partial application here
            break;
        }
    }

    auto call_expression = create_call(*compilation_state_,
        std::get<const DefinitionHeader*>(reference->value), std::vector<Expression*>{args}, reference->location);
    
    if (!call_expression) {
        Log::error(reference->location) << "Creating call expression failed" << Endl;
        return;
    }

    // determine the type
    if (args.size() == reference->type->arity()) {
        (*call_expression)->type = dynamic_cast<const FunctionType*>(reference->type)->return_type();
    } else {
        // TODO: partial application type
    }

    parse_stack_.push_back(*call_expression);
}

void TermedExpressionParser::partial_call_state() {
    if (at_expression_end())
        return;

    Expression* original = current_term(); // for the assertion at the bottom

    auto& [callee, args] = current_term()->call_value();

    // parse args
    for (unsigned int i = 0; i < callee->get_type()->arity(); i++) {
        if (at_expression_end())
            return;
        
        // handle missing args left in args list
        if (i < args.size()) {
            if (args.at(i)->expression_type == ExpressionType::missing_arg) {
                shift();
                Expression* new_arg = handle_arg_state(callee, args);
                ast_store_->delete_expression(args.at(i));
                args.at(i) = new_arg;
            }
            continue;
        }

        shift();
        args.push_back(handle_arg_state(callee, args));
    }
    // expression should already be the one on top
    assert(current_term() == original && "partial_call_state didn't maintain the current_term");
    return;
}

void TermedExpressionParser::deferred_call_state() {
    if (at_expression_end())
        return;

    switch (peek()->expression_type) {
        case GUARANTEED_VALUE:

        case ExpressionType::binary_operator_reference:
        case ExpressionType::minus_sign:
        case ExpressionType::call:
        case ExpressionType::reference:
        case TYPE_DECLARATION_TERM:
            assert(false && "Deferred call state not implemented");

        default:
            break;
    }

    Log::error(peek()->location) << "Unexpected " << *peek() << 
        ", expected an argument, operator or a type declaration";
    return fail();
}

Expression* TermedExpressionParser::handle_arg_state(
    const DefinitionHeader* callee, const std::vector<Expression*>& args) { 
    
    switch (current_term()->expression_type) {
        case ExpressionType::layer2_expression:
            handle_termed_sub_expression(current_term());
            return handle_arg_state(callee, args);
            
        // case ExpressionType::operator_reference:
            // abort, partial application
            // might be unary
            // !!! we might need to switch to peeking, unless we want to reverse

        case GUARANTEED_VALUE:
        case ExpressionType::reference:
        case ExpressionType::call:
            if (!is_acceptable_next_arg(callee, args/*, current_term()*/)) {
                // TODO: try all kinds of partial application
                Log::error(current_term()->location) << "possible type-error" << Endl;
                fail();
                return *pop_term();
            }

            return *pop_term();
        
        default:
            assert(false && "unhandled expression type in call_expression_state");
            return *pop_term();
    }
}

Expression* TermedExpressionParser::binary_minus_ref(SourceLocation location) {
    return create_operator_reference(
        *ast_store_, compilation_state_->special_definitions_.binary_minus, location);
}

Expression* TermedExpressionParser::unary_minus_ref(SourceLocation location) {
    return create_operator_reference(
        *ast_store_, compilation_state_->special_definitions_.unary_minus, location);
}

} // namespace Maps