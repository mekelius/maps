#include "parser_layer2.hh"

#include <cassert>
#include <compare>
#include <memory>
#include <span>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/compilation_state.hh"

#include "mapsc/types/type.hh"
#include "mapsc/types/function_type.hh"

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/callable.hh"
#include "mapsc/ast/ast_store.hh"


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
                         case ExpressionType::syntax_error:\
                         case ExpressionType::not_implemented:\
                         case ExpressionType::compiler_error

#define TYPE_DECLARATION_TERM ExpressionType::type_argument:\
                         case ExpressionType::type_field_name:\
                         case ExpressionType::type_constructor_reference:\
                         case ExpressionType::type_reference:\
                         case ExpressionType::type_construct

// Expression types guaranteed to be simple values
#define GUARANTEED_VALUE ExpressionType::string_literal:\
                    case ExpressionType::numeric_literal:\
                    case ExpressionType::value

#define POTENTIAL_FUNCTION ExpressionType::call:\
                      case ExpressionType::reference:\
                      case ExpressionType::termed_expression:\
                      case ExpressionType::ternary_expression

#define GUARANTEED_NON_OPERATOR_FUNCTION ExpressionType::lambda:\
                       case ExpressionType::partial_call

ParserLayer2::ParserLayer2(CompilationState* compilation_state)
:compilation_state_(compilation_state) {
}

void ParserLayer2::run() {
    for (Expression* expression: compilation_state_->unparsed_termed_expressions_) {
        // some expressions might be parsed early as sub-expressions
        if (expression->expression_type != ExpressionType::termed_expression)
            continue;
        
        TermedExpressionParser{compilation_state_, expression}.run();
    }
}

TermedExpressionParser::TermedExpressionParser(
    CompilationState* compilation_state, Expression* expression)
:expression_(expression), 
 compilation_state_(compilation_state), 
 ast_store_(compilation_state->ast_store_.get()) {
    expression_terms_ = &expression->terms();
    next_term_it_ = expression_terms_->begin();    
}

void TermedExpressionParser::run() {
    Expression* result = parse_termed_expression();

    // overwrite the expression in-place
    *expression_ = *result;

    Log::debug_extra("parsed a termed expression", result->location);
}

Expression* TermedExpressionParser::get_term() {
    Expression* current_term = *next_term_it_;
    assert(current_term->is_ok_in_layer2() && 
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
    assert(peek()->expression_type == ExpressionType::binary_operator_reference && 
        "TermedExpressionParser::peek_precedence called with not a binary operator on top of the stack");

    return get_operator_precedence(*peek());
}


void TermedExpressionParser::shift() {
    parse_stack_.push_back(get_term());
}

std::optional<Expression*> TermedExpressionParser::pop_term() {
    if (parse_stack_.size() == 0)
        return std::nullopt;

    Expression* expression = parse_stack_.back();
    parse_stack_.pop_back();
    return expression;
}

void TermedExpressionParser::fail(const std::string& message, SourceLocation location) {
    Log::error(message, location);
    compilation_state_->declare_invalid();
}

bool TermedExpressionParser::at_expression_end() const {
    return next_term_it_ >= expression_terms_->end();
}

bool TermedExpressionParser::parse_stack_reduced() const {
    // quick and dirty
    // TODO: some better check
    return parse_stack_.size() == 1;
}

// TODO: mave this to AST
bool is_value_literal(Expression* expression) {
    switch (expression->expression_type) {
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
            return true;
        
        default:
            return false;
    }
}

Expression* TermedExpressionParser::parse_termed_expression() {
    if (at_expression_end()) {
        fail("layer2 tried to parse an empty expression", expression_->location);
        return Expression::valueless(*ast_store_, ExpressionType::syntax_error, expression_->location);
    }

    // enter initial goto to find the first state
    shift();
    initial_goto();

    if (!compilation_state_->is_valid) {
        Log::error("parsing termed expression failed", expression_->location);
        return Expression::valueless(*ast_store_, ExpressionType::syntax_error, expression_->location);
    }

    if (!at_expression_end()) {
        assert(false && "parse_termed_expression didn't parse the whole expression");
        fail("parse_termed_expression didn't parse the whole expression", expression_->location);
        return expression_;
    }
    
    if (!parse_stack_reduced()) {
        assert(false && "parse_termed_expression failed to reduce completely");
        fail("parse_termed_expression failed to reduce the stack", expression_->location);
        return expression_;
    }

    return parse_stack_.back();
}

Expression* TermedExpressionParser::handle_termed_sub_expression(Expression* expression) {
    assert(expression->expression_type == ExpressionType::termed_expression 
        && "handle_sub_termed_expression called with non-termed expression");

    // TODO: pass some kind of type hint
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
    Expression* missing_argument = Expression::missing_argument(*ast_store_,
        *dynamic_cast<const FunctionType*>(op->type)->param_types().begin(), op->location);

    auto call =
        Expression::partial_binop_call(*compilation_state_,
            op->reference_value(), missing_argument, rhs, expression_->location);
    
    if (!call)
        return fail("Creating call expression failed", op->location);

    parse_stack_.push_back(*call);
}

void TermedExpressionParser::reduce_to_partial_binop_call_right() {
    Expression* op = *pop_term();
    Expression* lhs = *pop_term();

    // TODO: handle precedence here
    Expression* missing_argument = Expression::missing_argument(*ast_store_,
        *dynamic_cast<const FunctionType*>(op->type)->param_types().begin(), op->location);

    auto call =
        Expression::partial_binop_call(*compilation_state_,
            op->reference_value(), lhs, missing_argument, expression_->location);
    
    if (!call)
        return fail("Creating call expression failed", op->location);

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

    auto expression = Expression::call(*compilation_state_, op->reference_value(), 
        {value}, op->location);

    if (!expression)
        return fail("Applying unary prefix " + op->log_message_string() + " to " + 
            value->log_message_string() + " failed", op->location);

    parse_stack_.push_back(*expression);
}

void TermedExpressionParser::reduce_postfix_operator() {
    auto op = *pop_term();
    auto value = *pop_term();

    assert(op->expression_type == ExpressionType::postfix_operator_reference && 
        "reduce_postfix_operator called with not a postfix operator 1nd on the stack");

    auto expression = Expression::call(*compilation_state_, op->reference_value(), 
        {value}, op->location);

    if (!expression)
        return fail("Applying unary postfix " + op->log_message_string() + " to " + 
            value->log_message_string() + " failed", op->location);

    parse_stack_.push_back(*expression);
}

void TermedExpressionParser::reduce_partially_applied_minus() {
    auto arg = *pop_term();

    assert(current_term()->expression_type == ExpressionType::minus_sign && 
        "reduce_partially_applied_minus called with not a minus sign 2nd on the stack");
    auto location = (*pop_term())->location;
    auto expression = Expression::partially_applied_minus(
        *ast_store_, arg, location);
    return parse_stack_.push_back(expression);
}

void TermedExpressionParser::initial_goto() {
    switch (current_term()->expression_type) {
        case ExpressionType::termed_expression:
            handle_termed_sub_expression(current_term());
            return initial_goto();

        case ExpressionType::lambda:
            assert(false && "not implemented");
            // initial_callable_state();
            break;

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
            // TODO: make expressions print out nice
            Log::error("bad term type: " + std::to_string(static_cast<int>(peek()->expression_type)), 
                expression_->location);
            assert(false && "bad term in TermedExpressionParser::parse_termed_expression");
    }
}

void TermedExpressionParser::initial_binary_operator_state() {
    if (at_expression_end())
        return;        

    switch (peek()->expression_type) {
        case ExpressionType::termed_expression:
            handle_termed_sub_expression(peek());
            return initial_binary_operator_state();

        case ExpressionType::lambda:
        case ExpressionType::ternary_expression:
        case ExpressionType::partial_binop_call_both:
            assert(false && "not implemented");

        case GUARANTEED_VALUE:
            precedence_stack_.push_back(
                dynamic_cast<Operator*>(current_term()->operator_reference_value())
                    ->precedence());
            shift();
            initial_value_state();

            // TODO: look at the precedence stack

            switch (current_term()->expression_type) {
                case GUARANTEED_VALUE:
                case POTENTIAL_FUNCTION:
                    reduce_to_partial_binop_call_left();
                    return initial_partial_binop_call_left_state();

                case ExpressionType::partial_binop_call_right:
                    reduce_to_partial_binop_call_both();
                    return initial_partial_binop_call_both_state();

                default:
                    return fail("Unexpected " + current_term()->log_message_string() + 
                        ", expected a value", current_term()->location);
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
            initial_prefix_operator_state();
            auto location = current_term()->location;
            if (auto value = pop_term()) {
                auto missing_arg_type = 
                    dynamic_cast<const FunctionType*>(
                        current_term()->operator_reference_value()->get_type())
                            ->param_type(1);
                return push_partial_call(*pop_term(), 
                    {Expression::missing_argument(*ast_store_, *missing_arg_type, location), *value});
            }
            return fail("unary operator failed to produce a value", location);
        }

        case ExpressionType::postfix_operator_reference:
            return fail("Postfix unary operator " + peek()->log_message_string() + 
                " not allowed to operate on " + current_term()->log_message_string(), 
                peek()->location);

        case ExpressionType::reference:
            shift();
            initial_reference_state();
            switch (current_term()->expression_type) {
                case GUARANTEED_VALUE:
                case ExpressionType::reference:
                case ExpressionType::call:
                    return reduce_to_partial_binop_call_left();
                
                default:
                    fail("Unexpected " + current_term()->log_message_string() + 
                        " as right side of binary operator, expected a value", current_term()->location);
            }

        case ExpressionType::call:
            shift();
            initial_call_state();
            switch (current_term()->expression_type) {
                case GUARANTEED_VALUE:
                case ExpressionType::reference:
                case ExpressionType::call:
                    return reduce_to_partial_binop_call_left();

                default:
                    fail("Unexpected " + current_term()->log_message_string() + 
                        " as right side of binary operator, expected a value", current_term()->location);
            }

        case TYPE_DECLARATION_TERM:
            assert(false && "not implemented");

        case NOT_ALLOWED_IN_LAYER2:
            assert(false && "bad term encountered in TermedExpressoinParser");
    }
}

void TermedExpressionParser::initial_prefix_operator_state() {
    if (at_expression_end()) {
        auto op = *pop_term();
        return parse_stack_.push_back(Expression::reference(
            *ast_store_, op->operator_reference_value(), op->location));
    }

    prefix_operator_state();
    return initial_goto();
}

void TermedExpressionParser::initial_postfix_operator_state() {
    if (at_expression_end()) {
        auto op = *pop_term();
        return parse_stack_.push_back(Expression::reference(
            *ast_store_, op->operator_reference_value(), op->location));
    }

    return fail("Unexpected postfix operator " + current_term()->log_message_string() + 
        "at the start of an expression", current_term()->location);
}

void TermedExpressionParser::initial_minus_sign_state() {
    if (at_expression_end()) {
        auto declared_type = current_term()->declared_type;

        if (!declared_type)
            return fail("minus sign without arguments or declared type is ambiguous", 
                current_term()->location);

        if (declared_type == &Int_to_Int)
            return reduce_to_unary_minus_ref();

        if (declared_type == &IntInt_to_Int)
            return parse_stack_.push_back(binary_minus_ref(current_term()->location));

        return fail(
            "Type " + (*declared_type)->to_string() + " is not allowed with \"-\"", 
            current_term()->location);
    }

    switch (peek()->expression_type) {
        case ExpressionType::termed_expression:
            handle_termed_sub_expression(peek());
            return initial_minus_sign_state();

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
                    return initial_call_state();
                }

                case ExpressionType::minus_sign:
                    reduce_minus_sign_to_unary_minus_call();
                    parse_stack_.push_back(binary_minus_ref(get_term()->location));
                    return post_binary_operator_state();

                case TYPE_DECLARATION_TERM:
                    assert(false && "Type declaration after a minus term not implemented");

                default:
                    return fail("Unexpected " + peek()->log_message_string() + 
                    " after a value, expected an operator", peek()->location);
            }

            return initial_value_state();
        }

        case ExpressionType::minus_sign:
            shift();
            initial_minus_sign_state();
            reduce_partially_applied_minus();
            return initial_goto();

        case ExpressionType::partially_applied_minus:
            shift();
            current_term()->convert_to_unary_minus_call();
            reduce_partially_applied_minus();
            return initial_goto();

        case ExpressionType::reference:
        case ExpressionType::call:
            shift();
            reduce_partially_applied_minus();
            return initial_partially_applied_minus_state();

        case TYPE_DECLARATION_TERM:
        default:
            return fail("Initial minus sign not allowed with " + peek()->log_message_string(), 
                current_term()->location);
    }
}

void TermedExpressionParser::initial_partially_applied_minus_state() {
    if (at_expression_end())
        return;

    current_term()->convert_to_unary_minus_call();
    return initial_call_state();
}

// Basically we just unwrap it
void TermedExpressionParser::initial_partial_binop_call_right_state() {
    if (at_expression_end())
        return;

    auto location = current_term()->location;
    auto [callee, args] = current_term()->call_value();
    assert(callee->is_operator() 
        && "initial_partial_binop_call_right_state called with not an operator call");
    assert(dynamic_cast<Operator*>(callee)->is_binary() && 
        "initial_partial_binop_call_right_state called with not a binary operator call");

    assert(((args.size() == 2 && args.at(1)->expression_type == ExpressionType::missing_arg) || 
            args.size()) == 1 && 
                "initial_partial_binop_call_right_state called with invalid args");

    // auto precedence = dynamic_cast<Operator*>(callee)->precedence();

    switch (peek()->expression_type) {
        case ExpressionType::binary_operator_reference:
            current_term()->convert_to_partial_call(); // treat it as a value
            shift();
            return post_binary_operator_state();

        case ExpressionType::postfix_operator_reference:
            current_term()->convert_to_partial_call();
            shift();
            reduce_postfix_operator(); // It could be an operator that can act on this partial call
            return initial_goto();

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
            return fail("Unexpected " + peek()->log_message_string() + ", expected a value",
            peek()->location);
    }

    pop_term();
    parse_stack_.push_back(args.at(0));
    parse_stack_.push_back(Expression::operator_reference(*ast_store_, callee, location));
    return post_binary_operator_state();
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

void TermedExpressionParser::reference_state() {
    if (at_expression_end())
        return;

    // if it's not a function it's treated as a value
    // i.e. the next term has to be an operator
    if (current_term()->type->arity() == 0)
        return value_state();

    switch (peek()->expression_type) {
        case ExpressionType::termed_expression:
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
    if (at_expression_end())
        return;

    switch (peek()->expression_type) {
        case ExpressionType::lambda:
        case ExpressionType::ternary_expression:
            assert(false && "not implemented");

        case ExpressionType::termed_expression:
            handle_termed_sub_expression(peek());
            return value_state();

        case ExpressionType::partial_binop_call_both:
            assert(false && "not implemented");

        case ExpressionType::binary_operator_reference: {
            shift();
            auto op = current_term()->operator_reference_value();
            
            precedence_stack_.push_back(dynamic_cast<Operator*>(op)->precedence());
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
                dynamic_cast<Operator*>(current_term()->operator_reference_value())->precedence());
            return post_binary_operator_state();
        }

        case ExpressionType::reference:
        case ExpressionType::call:
        case GUARANTEED_VALUE:
            return fail("unexpected " + peek()->log_message_string() +
                " in termed expression, expected an operator", peek()->location);

        case ExpressionType::partially_applied_minus:
            peek()->convert_to_partial_binop_minus_call_left(*ast_store_);
            // intentional fall-through
        case ExpressionType::partial_binop_call_left: {
            auto location = current_term()->location;
            add_to_partial_call_and_push(get_term(), {*pop_term()}, location);
            return call_state();
        }
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_call:
            return fail("Unexpected partial call, expected an operator", peek()->location);

        case ExpressionType::type_reference:
        case ExpressionType::type_argument:
        case ExpressionType::type_construct:
        case ExpressionType::type_constructor_reference:
            assert(false && "not implemented");
    
        case ExpressionType::type_field_name:
        case NOT_ALLOWED_IN_LAYER2:
            // TODO: make expression to_str
            fail("bad term "+ peek()->log_message_string() + 
                " in initial value state", peek()->location);
            return;
    }
}

void TermedExpressionParser::prefix_operator_state() {
    if (at_expression_end()) {
        auto op = *pop_term();
        return parse_stack_.push_back(Expression::reference(
            *ast_store_, op->operator_reference_value(), op->location));
    }

    switch (peek()->expression_type) {
        case ExpressionType::termed_expression:
            handle_termed_sub_expression(peek());
            return prefix_operator_state();

        case GUARANTEED_VALUE:
            return push_unary_operator_call(*pop_term(), get_term());

        case ExpressionType::prefix_operator_reference: {
            auto location = current_term()->location;
            shift();
            prefix_operator_state();

            reduce_prefix_operator();

            return;
        }

        case ExpressionType::postfix_operator_reference:
        case NOT_ALLOWED_IN_LAYER2:
            return fail("Unexpected " + peek()->log_message_string() + " in termed expression", 
                peek()->location);
            
        case ExpressionType::reference:
        case ExpressionType::call:
            shift();
            reduce_prefix_operator();

            return;

        default:
            return fail("Unexpected " + current_term()->log_message_string() + 
            " after an unary operator", current_term()->location);
    }
    return fail("Unexpected " + current_term()->log_message_string() + 
    " after an unary operator", current_term()->location);
}

void TermedExpressionParser::call_state() {
    if (at_expression_end())
        return;

    auto type_to_use = current_term()->declared_type ?
        *current_term()->declared_type : current_term()->type;

    if (*type_to_use == Hole) {
        switch (peek()->expression_type) {
            default:
                assert(false && "Initial call type inference not implemented");
        }
    }

    // known non-nullary function
    if (type_to_use->arity() > 0)
        return deferred_call_state();

    // known nullary type
    return initial_value_state();
}

void TermedExpressionParser::partial_binop_call_standoff_state() {
    assert(false && "not implemented");
}

void TermedExpressionParser::post_binary_operator_state() {
    if (at_expression_end())
        return reduce_to_partial_binop_call_right();

    switch (peek()->expression_type) {
        case ExpressionType::termed_expression:
            handle_termed_sub_expression(peek());
            return post_binary_operator_state();

        case ExpressionType::binary_operator_reference:
        case ExpressionType::prefix_operator_reference:
        case ExpressionType::postfix_operator_reference:
        case ExpressionType::lambda:
        case ExpressionType::ternary_expression:
            assert(false && "not implemented");

        case ExpressionType::reference:
            shift();
            // if the reference is to a function, go ahead and try to apply it first
            if (current_term()->reference_value()->get_type()->arity() > 0) {
                call_expression_state();
            }
            return compare_precedence_state();

        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::call: {
            shift();
            
            // if we have a complete call, we can treat it as a value
            // if the call is partial, try to complete it first
            // NOTE: partial call_state could also return a partial call, in which case
            //       we can try to apply the operator onto that
            if (current_term()->is_partial_call()) {
                partial_call_state();
            }
            
            return compare_precedence_state();
        }

        case ExpressionType::minus_sign:
            shift();
            initial_minus_sign_state();
            return compare_precedence_state();

        case ExpressionType::partially_applied_minus:
            shift();
            current_term()->convert_to_unary_minus_call();
            return compare_precedence_state();

        case ExpressionType::partial_binop_call_both:
        case ExpressionType::partial_call:
            assert(false && "not implemented");
            
        case GUARANTEED_VALUE:
            shift();
            return compare_precedence_state();

        case TYPE_DECLARATION_TERM:
            if (peek()->expression_type == ExpressionType::type_reference) {
                shift();
                initial_type_reference_state();
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
    if (at_expression_end()) {
        reduce_binop_call();
        precedence_stack_.pop_back();
        return;
    }

    switch (peek()->expression_type) {
        case ExpressionType::binary_operator_reference:
        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_both:
        case ExpressionType::partially_applied_minus:
            break;

        case ExpressionType::minus_sign:
            peek()->convert_to_operator_reference(
                compilation_state_->special_callables_.binary_minus);
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

// Pops 3 values from the parse stack, reduces them into a binop apply expression and pushes it on top
void TermedExpressionParser::reduce_binop_call() {
    assert(parse_stack_.size() >= 3 
        && "TermedExpressionParser::reduce_operator_left_ called with parse stack size < 3");

    Expression* rhs = *pop_term();
    Expression* operator_ = *pop_term();
    Expression* lhs = *pop_term();

    // TODO: check types here
    assert(std::holds_alternative<Callable*>(operator_->value) && 
        "TermedExpressionParser::reduce_operator_left called with a call stack \
where operator didn't hold a reference to a callable");

    auto reduced = Expression::call(*compilation_state_,
        std::get<Callable*>(operator_->value), {lhs, rhs}, lhs->location);

    if (!reduced)
        return fail("Creating a binary operator call failed", rhs->location);

    (*reduced)->value = CallExpressionValue{std::get<Callable*>(operator_->value), 
        std::vector<Expression*>{lhs, rhs}};

    parse_stack_.push_back(*reduced);
}

void TermedExpressionParser::reduce_minus_sign_to_unary_minus_call() {
    auto arg = pop_term();
    assert(arg && "reduce_unary_minus_call called with nothing on the stack");

    assert(current_term()->expression_type == ExpressionType::minus_sign && 
        "reduce_unary_minus_call called with no minus sign on the stack");
    auto location = (*pop_term())->location; // eat the minus

    auto call = Expression::call(*compilation_state_, &unary_minus_Int, {*arg}, location);
    if (!call)
        return fail("Creating unary call to - failed", location);

    parse_stack_.push_back(*call);
}

void TermedExpressionParser::initial_type_reference_state() {
    assert(current_term()->expression_type == ExpressionType::type_reference && 
        "TermedExpressionParser::type_specifier_state entered with not a type_specifier/type_reference \
on the stack");

    if (at_expression_end()) {
        if (!possibly_type_expression_) {
            fail("Unexpected type expression at expression end", current_term()->location);
        }
        return;
    }

    switch (peek()->expression_type) {
        case GUARANTEED_VALUE:
        case ExpressionType::call: {
            auto type_term = *pop_term();
            assert(std::holds_alternative<const Type*>(type_term->value) && "no type");
            auto type_value = std::get<const Type*>(type_term->value);
            ast_store_->delete_expression(type_term);

            shift();
            auto term = current_term();

            // ??? What if this fails
            term->type->cast_to(type_value, *term);
            term->declared_type = type_value;
            term->location = type_term->location;
            term->expression_type = ExpressionType::value;
            return initial_value_state();
        }

        case ExpressionType::reference: {
            auto type_term = *pop_term();
            auto type_value = type_term->value;
            ast_store_->delete_expression(type_term);
            
            assert(std::holds_alternative<const Type*>(type_value) && "no type");
            shift();
            current_term()->declared_type = get<const Type*>(type_value);
            current_term()->location = type_term->location;
            return initial_reference_state(); 
        }

        case ExpressionType::binary_operator_reference:{
            shift();
            initial_binary_operator_state();
            auto value = pop_term();
            return apply_type_declaration(*pop_term(), *value);
        }
        case ExpressionType::termed_expression:
            // parse and come back
            // 
        case ExpressionType::type_reference:
            // fail

        default:
            assert(false && "not implemented");

        case NOT_ALLOWED_IN_LAYER2:
            fail("bad term encountered in TermedExpressoinParser", current_term()->location);
            assert(false && "bad term encountered in TermedExpressoinParser");
            return;
    }
}

void TermedExpressionParser::push_partial_call(Expression* callee_ref, 
    const std::vector<Expression*>& args) {

    push_partial_call(callee_ref, args, callee_ref->location);
}
void TermedExpressionParser::push_partial_call(Expression* callee_ref, 
    const std::vector<Expression*>& args, SourceLocation location) {
    
    auto call = Expression::call(*compilation_state_, 
        callee_ref->reference_value(), std::vector<Expression*>{args}, location);
    
    if (!call)
        return fail("During layer2: Creating partial call failed", location);
        
    parse_stack_.push_back(*call);
}

void TermedExpressionParser::add_to_partial_call_and_push(Expression* partial_call, 
    const std::vector<Expression*>& args, SourceLocation location) {

    assert(partial_call->is_partial_call() && 
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

    auto expression = Expression::call(*compilation_state_, callee, std::move(new_args), 
        location);

    if (!expression)
        return fail("Creating partial call to " + callee->to_string() + " failed", location);

    return parse_stack_.push_back(*expression);
}

void TermedExpressionParser::push_unary_operator_call(Expression* operator_ref, Expression* value) {
    auto location = 
        dynamic_cast<Operator*>(
            operator_ref->operator_reference_value())->fixity() == Operator::Fixity::unary_prefix ?
            operator_ref->location : value->location;
    auto call = Expression::call(*compilation_state_, 
        operator_ref->reference_value(), { value }, location);
            
    if (!call)
        return fail("Creating a call to unary operator failed", location);

    parse_stack_.push_back(*call);

    return initial_value_state();
}

void TermedExpressionParser::apply_type_declaration(Expression* type_declaration, Expression* value) {
    assert(false && "not implemented");
}

bool TermedExpressionParser::is_acceptable_next_arg(Callable* callee, 
    const std::vector<Expression*>& args/*, Expression* next_arg*/) {
    if (args.size() >= callee->get_type()->arity())
        return false;

    // TODO: check type
    return true;
}

// does it make sense to use different style of logic here?
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

    auto call_expression = Expression::call(*compilation_state_,
        std::get<Callable*>(reference->value), std::vector<Expression*>{args}, reference->location);
    
    if (!call_expression)
        return Log::error("Creating call expression failed", reference->location);

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

    return fail("Unexpected " + peek()->log_message_string() + 
        ", expected an argument, operator or a type declaration", 
        peek()->location);
}

Expression* TermedExpressionParser::handle_arg_state(
    Callable* callee, const std::vector<Expression*>& args) { 
    
    switch (current_term()->expression_type) {
        case ExpressionType::termed_expression:
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
                fail("possible type-error", current_term()->location);
                return *pop_term();
            }

            return *pop_term();
        
        default:
            assert(false && "unhandled expression type in call_expression_state");
            return *pop_term();
    }
}

Expression* TermedExpressionParser::binary_minus_ref(SourceLocation location) {
    return Expression::operator_reference(
        *ast_store_, compilation_state_->special_callables_.binary_minus, location);
}

Expression* TermedExpressionParser::unary_minus_ref(SourceLocation location) {
    return Expression::operator_reference(
        *ast_store_, compilation_state_->special_callables_.unary_minus, location);
}

} // namespace Maps