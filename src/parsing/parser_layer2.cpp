#include <cassert>

#include "parser_layer2.hh"
#include "name_resolution.hh"

using Logging::log_error, Logging::log_info;
using AST::ExpressionType, AST::Expression;

// Expression types that are not allowed here
// NOTE: Empty is allowed at top-level
#define BAD_TERM ExpressionType::identifier: case ExpressionType::operator_e: case ExpressionType::deleted: case ExpressionType::missing_arg: case ExpressionType::syntax_error: case ExpressionType::not_implemented: case ExpressionType::empty: case ExpressionType::tie

// Expression types guaranteed to be simple values
#define GUARANTEED_VALUE ExpressionType::string_literal: case ExpressionType::numeric_literal

#define POTENTIAL_FUNCTION ExpressionType::call: case ExpressionType::reference: case ExpressionType::termed_expression

ParserLayer2::ParserLayer2(AST::AST* ast, Pragma::Pragmas* pragmas)
: ast_(ast), pragmas_(pragmas) {
}

void ParserLayer2::run() {
    for (Expression* expression: ast_->unparsed_termed_expressions) {
        SourceLocation original_location = expression->location;

        TermedExpressionParser{ast_, expression}.run();
        
        assert(expression->location == original_location 
            && "parsing termed expression changed the location of the expression");
    }
}

TermedExpressionParser::TermedExpressionParser(AST::AST* ast, Expression* expression)
:ast_(ast), expression_(expression) {
    expression_terms_ = &std::get<AST::TermedExpressionValue>(expression->value);
    next_term_it_ = expression_terms_->begin();
    
    next_term_ = (next_term_it_ != expression_terms_->end()) ? 
        *next_term_it_ : nullptr;    
}

void TermedExpressionParser::run() {
    Expression* result = parse_termed_expression();

    // overwrite the expression in-place
    *expression_ = *result;

    log_info(result->location, "parsed a termed expression",
        Logging::MessageType::parser_debug_termed_expression);
}

Expression* TermedExpressionParser::get_term() {
    current_term_ = next_term_;

    next_term_it_++;

    next_term_ = (next_term_it_ < expression_terms_->end()) ? 
        *next_term_it_ : nullptr;

    return current_term_;
}

AST::Expression* TermedExpressionParser::peek() const {
    return next_term_;
}

void TermedExpressionParser::shift() {
    parse_stack_.push_back(get_term());
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
        log_error(expression_->location, "layer2 tried to parse an empty expresison");
        ast_->declare_invalid();
        return ast_->create_valueless_expression(AST::ExpressionType::empty, expression_->location);
    }

    // safe to unwrap because at_expression_end was checked above
    switch (peek()->expression_type) {
        // terminals
        case ExpressionType::reference:
            shift();
            initial_identifier_state();
            break;

        case ExpressionType::call:
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
        case ExpressionType::termed_expression:
            shift();
            initial_value_state();
            break;

        case ExpressionType::operator_ref:
            shift();
            initial_operator_state();
            break;

        default:
            assert(false && "unhandled expressiontype in TermedExpressionParser::parse_termed_expression");
    }

    if (!at_expression_end()) {
        assert(false && "parse_termed_expression didn't parse the whole expression");
        log_error(expression_->location, 
            "parse_termed_expression didn't parse the whole expression");
        ast_->declare_invalid();
        return expression_;
    }
    
    if (!parse_stack_reduced()) {
        assert(false && "parse_termed_expression failed to reduce completely");
        log_error(expression_->location, 
            "parse_termed_expression failed to reduce the stack");
        ast_->declare_invalid();
        return expression_;
    }

    return parse_stack_.back();
}

AST::Expression* TermedExpressionParser::handle_sub_termed_expression(AST::Expression* expression) {
    assert(expression->expression_type == ExpressionType::termed_expression 
        && "handle_sub_termed_expression called with non-termed expression");

    // TODO: pass some kind of type hint
    TermedExpressionParser{ast_, expression}.run();

    return expression;
}


// ----- STATE FUNCTIONS -----

void TermedExpressionParser::initial_identifier_state() {
    if (at_expression_end()) {
        return;
    }

    // if it's not a function it's treated as a value
    // i.e. the next term has to be an operator
    if (current_term_->type.arity() == 0) {
        switch (peek()->expression_type) {
            case ExpressionType::operator_ref:
                precedence_stack_.push_back(peek()->type.precedence());
                shift();
                return post_binary_operator_state();                

            default:
                log_error(peek()->location, "unexpected non-operator in termed expression");
                ast_->declare_invalid();
                return;
        }
    }

    switch (peek()->expression_type) {
        case GUARANTEED_VALUE:
        case ExpressionType::reference:
        case ExpressionType::termed_expression:
        case ExpressionType::call:
            return call_expression_state();

        case ExpressionType::operator_ref:
            precedence_stack_.push_back(peek()->type.precedence());
            shift();
            return post_binary_operator_state();

        default:
            assert(false && "unhandled term type in TermedExpressionParser::initial_identifier_state");
    }
}

void TermedExpressionParser::initial_value_state() {
    if (at_expression_end()) {
        // single values just get returned
        return;
    }

    switch (peek()->expression_type) {
        case ExpressionType::operator_ref:
            precedence_stack_.push_back(peek()->type.precedence());
            shift();
            return post_binary_operator_state();

        default:
            // TODO: make expression to_str
            log_error(peek()->location, 
                "unexpected *something* after a value, expected an operator");
            ast_->declare_invalid();
            return;
    }
}

void TermedExpressionParser::initial_operator_state() {
    if (at_expression_end()) {
        return;
    }

    // TODO: check arity
    switch (peek()->expression_type) {
        case ExpressionType::termed_expression:
            handle_sub_termed_expression(peek());
            return initial_operator_state();

        case GUARANTEED_VALUE: {
            shift();
            if (at_expression_end()) {
                // just reduce
                AST::Expression* rhs = parse_stack_.back();
                parse_stack_.pop_back();
                AST::Expression* op = parse_stack_.back();
                parse_stack_.pop_back();

                AST::Expression* call = ast_->globals_->create_call_expression(op->reference_value(), {
                        ast_->create_valueless_expression(AST::ExpressionType::missing_arg, op->location), 
                        rhs },
                    expression_->location);

                parse_stack_.push_back(call);

                return;
            }
        }

        case ExpressionType::call:
        case ExpressionType::operator_ref:
        case ExpressionType::reference:
            assert(false && "not implemented");

        case BAD_TERM:
            assert(false && "bad term encountered in TermedExpressoinParser");
    }
}

void TermedExpressionParser::post_binary_operator_state() {
    if (at_expression_end()) {
        assert(false && "partial application not implemented");
        // handle partial application
        return;
    }

    switch (peek()->expression_type) {
        case ExpressionType::operator_ref:
            // assert that it's unary
            break;

        // value
        case ExpressionType::call:
        case ExpressionType::termed_expression:
        case ExpressionType::reference:
            assert(false && "not implemented");

        case GUARANTEED_VALUE:
            shift();
            return compare_precedence_state();

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
    if (precedence_stack_.back() > peek()->type.precedence()) {
        reduce_operator_left();
        precedence_stack_.pop_back();
        
        // if there's an "intermediate level" or operators, we need to handle those before returning
        if (precedence_stack_.back() < peek()->type.precedence()) {
            precedence_stack_.push_back(peek()->type.precedence());
            shift();
            return post_binary_operator_state();
        }
        
        // otherwise simply return 
        return;
    }

    assert(peek()->expression_type == ExpressionType::operator_ref
        && "compare_precedence_state called with not an operator ref next");

    if (precedence_stack_.back() == peek()->type.precedence()) { // !!!: assuming left-associativity
        // if the precedence stays the same, just reduce and carry on
        reduce_operator_left();
        shift();
        return post_binary_operator_state();
    }

    // if the precedence goes up, push it into the stack and run post_binary_op_state with the new precedence stack
    // once it returns, there should be a nice reduced rhs on the parse_stack_
    if (precedence_stack_.back() < peek()->type.precedence()) {
        unsigned int previous_precedence = precedence_stack_.back();
        precedence_stack_.push_back(peek()->type.precedence());
        shift();
        post_binary_operator_state();
        assert(precedence_stack_.back() == previous_precedence 
            && "post_binary_operator_state didn't return to the same precedence level");
            
        reduce_operator_left();

        if (at_expression_end()) {
            // ??? should we look at the precedence stack here? 
            return;
        }

        assert(peek()->type.precedence() >= precedence_stack_.back() 
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

    Expression* rhs = parse_stack_.back();
    parse_stack_.pop_back();

    Expression* operator_ = parse_stack_.back();
    parse_stack_.pop_back();

    Expression* lhs = parse_stack_.back();
    parse_stack_.pop_back();

    // TODO: check types here
    assert(std::holds_alternative<AST::Callable*>(operator_->value) 
        && "TermedExpressionParser::reduce_operator_left called with a call stack where operator didn't hold a reference to a callable");

    Expression* reduced = ast_->globals_->create_call_expression(
        std::get<AST::Callable*>(operator_->value), {lhs, rhs}, lhs->location);
    reduced->value = AST::CallExpressionValue{std::get<AST::Callable*>(operator_->value), std::vector<AST::Expression*>{lhs, rhs}};

    parse_stack_.push_back(reduced);
}

void TermedExpressionParser::unary_operators_state() {
    if (at_expression_end()) {
        return;
    }
}

bool TermedExpressionParser::is_acceptable_next_arg(AST::Expression* callee, 
    const std::vector<AST::Expression*>& args, AST::Expression* next_arg) {
    if (args.size() >= callee->type.arity())
        return false;

    // TODO: check type
    return true;
}

void TermedExpressionParser::call_expression_state() {
    AST::Expression* callee = current_term_;
    
    assert(callee->expression_type == AST::ExpressionType::reference 
        && "TermedExpressionParser called with a callee that was not a reference");
    assert(callee->type.arity() > 0 && "call_expression_state cassed with arity 0");

    std::vector<AST::Expression*> args;
    
    // parse args
    for (int i = callee->type.arity(); i > 0; i--) {
        AST::Expression* next = get_term();

        switch (next->expression_type) {
            case ExpressionType::operator_ref:
                // abort, partial application
                // might be unary
                // !!! we might need to switch to peeking, unless we want to reverse

            case GUARANTEED_VALUE:
            case ExpressionType::reference:
            case ExpressionType::call:
                if (!is_acceptable_next_arg(callee, args, next)) {
                    // TODO: try all kinds of partial application
                    log_error(next->location, "possible type-error");
                    ast_->declare_invalid();
                    return;
                }
                args.push_back(next);
                break;
            
            default:
                assert(false && "unhandled expression type in call_expression_state");
        }

        if (at_expression_end()) {
            // accept partial application here
            break;
        }
    }

    parse_stack_.pop_back();
    AST::Expression* call_expression = ast_->globals_->create_call_expression(
        std::get<AST::Callable*>(callee->value), args, callee->location);
    
    // determine the type
    if (args.size() == callee->type.arity()) {
        assert(std::holds_alternative<std::unique_ptr<AST::FunctionTypeComplex>>(callee->type.complex) 
            && "non-function type callee in call_expression_state");
        call_expression->type = callee->type.function_type()->return_type;
    } else {
        // TODO: partial application type
    }

    parse_stack_.push_back(call_expression);
}
