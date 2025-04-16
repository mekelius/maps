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

        case ExpressionType::builtin_operator:
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

}

void TermedExpressionParser::initial_value_state() {
    shift();
    if (at_expression_end()) {
        return;
    }

    switch (peek()->expression_type) {
        case ExpressionType::tie:
            return parse_tied_operator();

        case ExpressionType::builtin_operator:
            return post_binary_operator_state();

        default:
            // TODO: make expression to_str
            log_error(peek()->location, "unexpected *something* after a value, expected an operator");
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
    shift();
    if (at_expression_end()) {
        return;
    }

    switch (peek()->expression_type) {
        case ExpressionType::builtin_operator:
            // assert that it's unary
            break;
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
    shift();
    if (at_expression_end()) {
        reduce_operator_left();
        return;
    }

    switch (peek()->expression_type) {
        case ExpressionType::builtin_operator:
            reduce_operator_left();
        default:
    }
}

void TermedExpressionParser::arg_list_state() {
    shift();
    if (at_expression_end()) {
        return;
    }
}

void TermedExpressionParser::unary_operators_state() {
    shift();
    if (at_expression_end()) {
        return;
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

    // TODO: have this on the operator expression value
    std::optional<AST::Callable*> operator_callable = ast_->builtin_operators_.get_identifier(operator_->string_value());
    assert(operator_callable && "somehow a non-existent operator got past name resolution step to ParseLayer2::reduce_operator_left");

    Expression* lhs = parse_stack_.back();
    parse_stack_.pop_back();

    // TODO: check types here

    Expression* reduced = ast_->create_expression(ExpressionType::binary_operator_apply, lhs->location);
    reduced->value = AST::BinaryOperatorApplyValue{*operator_callable, lhs, rhs};

    parse_stack_.push_back(reduced);
}