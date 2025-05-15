#include "parser_layer2.hh"

#include <cassert>
#include <compare>
#include <memory>
#include <span>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"
#include "mapsc/loglevel_defs.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/types/type.hh"
#include "mapsc/types/function_type.hh"

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/callable.hh"
#include "mapsc/ast/ast_store.hh"

using Maps::GlobalLogger::log_error, Maps::GlobalLogger::log_info;

namespace Maps {

// Expression types that are not allowed here
// NOTE: Empty is allowed at top-level
#define BAD_TERM ExpressionType::identifier: \
            case ExpressionType::type_identifier: \
            case ExpressionType::operator_identifier: \
            case ExpressionType::type_operator_identifier: \
            case ExpressionType::deleted: \
            case ExpressionType::missing_arg: \
            case ExpressionType::syntax_error: \
            case ExpressionType::not_implemented

#define TYPE_DECLARATION_TERM ExpressionType::type_argument: \
                         case ExpressionType::type_field_name: \
                         case ExpressionType::type_constructor_reference: \
                         case ExpressionType::type_reference: \
                         case ExpressionType::type_operator_reference: \
                         case ExpressionType::type_construct

// Expression types guaranteed to be simple values
#define GUARANTEED_VALUE ExpressionType::string_literal: \
                    case ExpressionType::numeric_literal: \
                    case ExpressionType::value

#define POTENTIAL_FUNCTION ExpressionType::call: \
                      case ExpressionType::reference: \
                      case ExpressionType::termed_expression

ParserLayer2::ParserLayer2(CompilationState* compilation_state)
: compilation_state_(compilation_state) {
}

void ParserLayer2::run() {
    for (Expression* expression: compilation_state_->unparsed_termed_expressions_) {
        // some expressions might be parsed early as sub-expressions
        if (expression->expression_type != ExpressionType::termed_expression)
            continue;
        
        TermedExpressionParser{compilation_state_, expression}.run();
    }
}

TermedExpressionParser::TermedExpressionParser(CompilationState* compilation_state, Expression* expression)
:expression_(expression), compilation_state_(compilation_state), ast_store_(compilation_state->ast_store_.get()) {
    expression_terms_ = &expression->terms();
    next_term_it_ = expression_terms_->begin();    
}

void TermedExpressionParser::run() {
    Expression* result = parse_termed_expression();

    // overwrite the expression in-place
    *expression_ = *result;

    log_info("parsed a termed expression",
        MessageType::parser_debug_termed_expression, result->location);
}

Expression* TermedExpressionParser::get_term() {
    Expression* current_term = *next_term_it_;
    next_term_it_++;
    return current_term;
}

Expression* TermedExpressionParser::peek() const {
    assert(next_term_it_ != expression_terms_->end() && "peek called with no termas left");
    return *next_term_it_;
}

Expression* TermedExpressionParser::current_term() const {
    assert(!parse_stack_.empty() && "tried to read current_term from an empty parse stack");
    return parse_stack_.back();
}

Precedence TermedExpressionParser::peek_precedence() const {
    assert(peek()->expression_type == ExpressionType::operator_reference && 
        "TermedExpressionParser::peek_precedence called with not an operator on top of the stack");

    assert(peek()->reference_value()->is_binary_operator() && 
        "TermedExpressionParser::peek_precedence encountered an operator ref that was not a binary operator");

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
    log_error(message, location);
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
        return create_valueless_expression(*ast_store_, ExpressionType::syntax_error, expression_->location);
    }

    // safe to unwrap because at_expression_end was checked above
    switch (peek()->expression_type) {
        case ExpressionType::termed_expression:
            handle_termed_sub_expression(peek());
            return parse_termed_expression();

        // terminals
        case ExpressionType::reference:
            shift();
            initial_reference_state();
            break;

        case ExpressionType::call:
        case GUARANTEED_VALUE:
            shift();
            initial_value_state();
            break;

        case ExpressionType::operator_reference:
            shift();
            initial_operator_state();
            break;

        case ExpressionType::type_reference:
        case ExpressionType::type_operator_reference:
            shift();
            initial_type_reference_state();
            break;

        case ExpressionType::type_field_name:
        case ExpressionType::type_argument:
        case ExpressionType::type_construct:
        case ExpressionType::type_constructor_reference:
        case BAD_TERM:
            // TODO: make expressions print out nice
            log_error("bad term type: " + std::to_string(static_cast<int>(peek()->expression_type)), 
                expression_->location);
            assert(false && "bad term in TermedExpressionParser::parse_termed_expression");
    }

    if (!compilation_state_->is_valid) {
        log_error("parsing termed expression failed", expression_->location);
        return create_valueless_expression(*ast_store_, ExpressionType::syntax_error, expression_->location);
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

    // if it's not a function it's treated as a value
    // i.e. the next term has to be an operator
    if (current_term()->type->arity() == 0) {
        switch (peek()->expression_type) {
            case ExpressionType::operator_reference:
                precedence_stack_.push_back(peek_precedence());
                shift();
                return post_binary_operator_state();                

            default:
                fail("unexpected non-operator in termed expression", peek()->location);
                return;
        }
    }

    switch (peek()->expression_type) {
        case ExpressionType::termed_expression:
            handle_termed_sub_expression(peek());
            return initial_reference_state();

        case GUARANTEED_VALUE:
        case ExpressionType::reference:
        case ExpressionType::call:
            return call_expression_state();

        case ExpressionType::operator_reference:
            precedence_stack_.push_back(peek_precedence());
            shift();
            return post_binary_operator_state();

        default:
            assert(false && "unhandled term type in TermedExpressionParser::initial_identifier_state");
    }
}

// current_term is a value, so next term has to be something else, or eventually reduce to a binop
void TermedExpressionParser::initial_value_state() {
    if (at_expression_end())
        return;

    switch (peek()->expression_type) {
        case ExpressionType::termed_expression:
            handle_termed_sub_expression(peek());
            return initial_value_state();

        case ExpressionType::operator_reference:
            precedence_stack_.push_back(peek_precedence());
            shift();
            return post_binary_operator_state();

        case ExpressionType::reference:
        case ExpressionType::call:
        case GUARANTEED_VALUE:
            return fail("unexpected value expression " + peek()->log_message_string() +
                " in termed expression, expected an operator", peek()->location);

        case ExpressionType::type_reference:
        case ExpressionType::type_operator_reference:
        case ExpressionType::type_argument:
        case ExpressionType::type_construct:
        case ExpressionType::type_constructor_reference:
            assert(false && "not implemented");
    
        case ExpressionType::type_field_name:
        case BAD_TERM:
            // TODO: make expression to_str
            fail("bad term "+ peek()->log_message_string() + " in initial value state", peek()->location);
            return;
    }
}

void TermedExpressionParser::initial_operator_state() {
    if (at_expression_end())
        return;        

    switch (peek()->expression_type) {
        case ExpressionType::termed_expression:
            handle_termed_sub_expression(peek());
            return initial_operator_state();

        case GUARANTEED_VALUE: {
            Expression* op = *pop_term();
            Expression* rhs = get_term();

            // if it's an unary operator, just apply and be done with it
            if (op->operator_reference_value()->is_unary_operator()) {
                auto call = create_call_expression(*ast_store_, expression_->location, 
                    op->reference_value(), { rhs });
                
                if (!call)
                    return fail("Creating call to unary operator failed", op->location);

                parse_stack_.push_back(*call);

                return initial_value_state();
            }

            // TODO: handle precedence here
            // it's a binary operator being partially applied
            Expression* missing_argument = create_missing_argument(*ast_store_, op->location,
                *dynamic_cast<const FunctionType*>(op->type)->param_types().begin());

            auto call =
                create_call_expression(*ast_store_, expression_->location,
                    op->reference_value(), { missing_argument, rhs });
            
            if (!call)
                return fail("Creating call expression failed", op->location);

            parse_stack_.push_back(*call);
            return;
        }

        case TYPE_DECLARATION_TERM:
        case ExpressionType::call:
        case ExpressionType::operator_reference:
        case ExpressionType::reference:
            assert(false && "not implemented");

        case BAD_TERM:
            assert(false && "bad term encountered in TermedExpressoinParser");
    }
}

void TermedExpressionParser::post_binary_operator_state() {
    if (at_expression_end()) {
        Expression* op = *pop_term(); // pop the operator
        Expression* lhs = *pop_term();
        Expression* missing_argument = create_missing_argument(*ast_store_, op->location,
            *dynamic_cast<const FunctionType*>(op->type)->param_types().begin());

        auto call = create_call_expression(*ast_store_, lhs->location, op->reference_value(), 
            {lhs, missing_argument});

        if (!call)
            return fail("Creating a call failed", lhs->location);

        parse_stack_.push_back(*call);
        return;
    }

    switch (peek()->expression_type) {
        case ExpressionType::termed_expression:
            handle_termed_sub_expression(peek());
            return post_binary_operator_state();

        case ExpressionType::operator_reference:
            // assert that it's unary
            break;

        case ExpressionType::reference:
            shift();
            // if the reference is to a function, go ahead and try to apply it first
            if (current_term()->reference_value()->get_type()->arity() > 0) {
                call_expression_state();
            }
            return compare_precedence_state();

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

        case BAD_TERM:
            assert(false && "bad term in TermedExpressionParser::post_binary_operator_state");
            break;
    }
}

// stack state: REDUCED_TREE OP1 VAL | input state: OP2(?)
// so, we compare the precedences of the operators
// if OP1 wins, we can reduce left, if OP2 wins we must wait (how?)
void TermedExpressionParser::compare_precedence_state() {
    if (at_expression_end()) {
        reduce_operator_left();
        precedence_stack_.pop_back();
        return;
    }

    // precedence goes down => one or more "closing parenthesis" required
    Precedence next_precedence = peek_precedence();

    if (precedence_stack_.back() > next_precedence) {
        reduce_operator_left();
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

    assert(peek()->expression_type == ExpressionType::operator_reference
        && "compare_precedence_state called with not an operator ref next");

    if (precedence_stack_.back() == next_precedence) { // !!!: assuming left-associativity
        // if the precedence stays the same, just reduce and carry on
        reduce_operator_left();
        shift();
        return post_binary_operator_state();
    }

    // if the precedence goes up, push it into the stack and run post_binary_op_state with the new precedence stack
    // once it returns, there should be a nice reduced rhs on the parse_stack_
    if (precedence_stack_.back() < next_precedence) {
        unsigned int previous_precedence = precedence_stack_.back();
        precedence_stack_.push_back(next_precedence);
        shift();
        post_binary_operator_state();
        assert(precedence_stack_.back() == previous_precedence 
            && "post_binary_operator_state didn't return to the same precedence level");
            
        reduce_operator_left();

        if (at_expression_end()) {
            // ??? should we look at the precedence stack here? 
            return;
        }

        assert(peek_precedence() >= precedence_stack_.back() 
            && "post_binary_operator_state didn't run until the end or a lover/equal precedence that the caller put on the precedence stack");
        
        // continue parsing
        shift();
        return post_binary_operator_state(); /// how about the shifts here
    }
}

// Pops 3 values from the parse stack, reduces them into a binop apply expression and pushes it on top
void TermedExpressionParser::reduce_operator_left() {
    assert(parse_stack_.size() >= 3 
        && "TermedExpressionParser::reduce_operator_left_ called with parse stack size < 3");

    Expression* rhs = *pop_term();
    Expression* operator_ = *pop_term();
    Expression* lhs = *pop_term();

    // TODO: check types here
    assert(std::holds_alternative<Callable*>(operator_->value) 
        && "TermedExpressionParser::reduce_operator_left called with a call stack where operator didn't hold a reference to a callable");

    auto reduced = create_call_expression(*ast_store_, lhs->location,
        std::get<Callable*>(operator_->value), {lhs, rhs});

    if (!reduced)
        return fail("Creating a binary operator call failed", rhs->location);

    (*reduced)->value = CallExpressionValue{std::get<Callable*>(operator_->value), 
        std::vector<Expression*>{lhs, rhs}};

    parse_stack_.push_back(*reduced);
}

void TermedExpressionParser::initial_type_reference_state() {
    assert(current_term()->expression_type == ExpressionType::type_reference && 
        "TermedExpressionParser::type_specifier_state entered with not a type_specifier/type_reference on the stack");

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
        case ExpressionType::operator_reference:
            
        case ExpressionType::termed_expression:
            // parse and come back
            // 
        case ExpressionType::type_operator_reference:
            // make a type construct

        case ExpressionType::type_reference:
            // fail

        default:
            assert(false && "-.-"); // !!!

        case BAD_TERM:
            fail("bad term encountered in TermedExpressoinParser", current_term()->location);
            assert(false && "bad term encountered in TermedExpressoinParser");
            return;
    }
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
    assert(reference->type->arity() > 0 && "call_expression_state cassed with arity 0");

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

    auto call_expression = create_call_expression(*ast_store_, reference->location,
        std::get<Callable*>(reference->value), args);
    
    if (!call_expression)
        return log_error("Creating call expression failed", reference->location);

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
        if (at_expression_end()) {
            return;
        }
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

Expression* TermedExpressionParser::handle_arg_state(Callable* callee, const std::vector<Expression*>& args) { 
    switch (current_term()->expression_type) {
        case ExpressionType::termed_expression:
            handle_termed_sub_expression(current_term());
            return handle_arg_state(callee, args);
            
        case ExpressionType::operator_reference:
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

} // namespace Maps