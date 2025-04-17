#include <cassert>

#include "parser_layer2.hh"
#include "name_resolution.hh"

using Logging::log_error, Logging::log_info;
using AST::ExpressionType, AST::Expression;

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
        return ast_->create_expression(AST::ExpressionType::empty, expression_->location);
    }

    // safe to unwrap because at_expression_end was checked above
    switch (peek()->expression_type) {
        // terminals
        case ExpressionType::identifier:
            initial_identifier_state();
            break;

        case ExpressionType::call:
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
        case ExpressionType::builtin_function:
        case ExpressionType::termed_expression:
            initial_value_state();
            break;

        case ExpressionType::operator_ref:
            // TODO: check if unary
            shift();
            break;

        case ExpressionType::deferred_call:
            shift();
            // TODO: try to resolve callee
            break;

        case ExpressionType::tie:
            log_error(expression_->location, "unexpected tie in termed expression");
            ast_->declare_invalid();
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

void TermedExpressionParser::parse_tied_operator() {
    log_error(expression_->location, "parse_tied_operator not implemented");
    ast_->declare_invalid();
}


// ----- STATE FUNCTIONS -----

void TermedExpressionParser::initial_identifier_state() {
    shift();
    if (at_expression_end()) {
        return;
    }

    // if it's not a function it's treated as a value
    // i.e. the next term has to be an operator (or a tie)
    if (current_term_->type.arity() == 0) {
        switch (peek()->expression_type) {
            case ExpressionType::operator_ref:
                precedence_stack_.push_back(peek()->type.precedence());
                return post_binary_operator_state();                

            case ExpressionType::tie:
                return parse_tied_operator();

            default:
                log_error(peek()->location, "unexpected non-operator in termed expression");
                ast_->declare_invalid();
                return;
        }
    }

    switch (peek()->expression_type) {
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
        case ExpressionType::identifier:
        case ExpressionType::termed_expression:
        case ExpressionType::call:
        case ExpressionType::deferred_call:
        case ExpressionType::binary_operator_apply:
            return call_expression_state();

        case ExpressionType::tie:
            parse_tied_operator();
            break;

        case ExpressionType::operator_ref:
            precedence_stack_.push_back(peek()->type.precedence());
            return post_binary_operator_state();
    }
}

void TermedExpressionParser::initial_value_state() {
    shift();
    if (at_expression_end()) {
        // single values just get returned
        return;
    }

    switch (peek()->expression_type) {
        case ExpressionType::tie:
            return parse_tied_operator();

        case ExpressionType::operator_ref:
            precedence_stack_.push_back(peek()->type.precedence());
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
    shift();
    if (at_expression_end()) {
        // might be partial application or postfix unary
        return;
    }

    // check arity
}

// maybe dumb?
void TermedExpressionParser::pre_binary_operator_state() {
    shift();
    if (at_expression_end()) {
        return;
    }

    
    // check type
    // compare precedence
}

void TermedExpressionParser::post_binary_operator_state() {
    shift(); // shift in the operator
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
        case ExpressionType::identifier:
        case ExpressionType::numeric_literal:
        case ExpressionType::string_literal:
            return compare_precedence_state();
    }

}

// stack state: REDUCED_TREE OP1 VAL | input state: OP2(?)
// so, we compare the precedences of the operators
// if OP1 wins, we can reduce left, if OP2 wins we must wait (how?)
void TermedExpressionParser::compare_precedence_state() {
    shift(); //!!! shift in an assumed value
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
        return post_binary_operator_state();
    }

    // if the precedence goes up, push it into the stack and run post_binary_op_state with the new precedence stack
    // once it returns, there should be a nice reduced rhs on the parse_stack_
    if (precedence_stack_.back() < peek()->type.precedence()) {
        unsigned int previous_precedence = precedence_stack_.back();
        precedence_stack_.push_back(peek()->type.precedence());
        
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

    Expression* reduced = ast_->create_expression(ExpressionType::binary_operator_apply, lhs->location);
    reduced->value = AST::BinaryOperatorApplyValue{std::get<AST::Callable*>(operator_->value), lhs, rhs};

    parse_stack_.push_back(reduced);
}

void TermedExpressionParser::unary_operators_state() {
    shift();
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

void TermedExpressionParser::arg_list_state() {
    shift();
    if (at_expression_end()) {
        return;
    }
}

void TermedExpressionParser::call_expression_state() {
    AST::Expression* callee = current_term_;
    
    // at this point the type should be set by name_resolution (or inference?)
    AST::Type callee_type = current_term_->type;
    assert(callee_type.arity() > 0 && "call_expression_state cassed with arity 0");

    std::vector<AST::Expression*> args;
    
    // parse args
    for (int i = callee_type.arity(); i > 0; i--) {
        AST::Expression* next = get_term();

        switch (next->expression_type) {
            case ExpressionType::tie:
                assert(false && "ties not implemented yet");
                parse_tied_operator();
                break;

            case ExpressionType::operator_ref:
                // abort, partial application
                // might be unary
                // !!! we might need to switch to peeking, unless we want to reverse

            case ExpressionType::numeric_literal:
            case ExpressionType::string_literal:
            case ExpressionType::binary_operator_apply:
            case ExpressionType::identifier:
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
    AST::Expression* call_expression = ast_->create_expression(
        ExpressionType::call, {callee->location});
    call_expression->value = AST::CallExpressionValue{callee->string_value(), args};
    
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

AST::Expression* TermedExpressionParser::parse_call_expression() {
}