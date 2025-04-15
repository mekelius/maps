#include <cassert>

#include "parser_layer2.hh"
#include "name_resolution.hh"

using Logging::log_error, Logging::log_info;
using AST::ExpressionType, AST::Expression;

ParserLayer2::ParserLayer2(AST::AST* ast, Pragma::Pragmas* pragmas)
: ast_(ast), pragmas_(pragmas) {
}

void ParserLayer2::run() {
    resolve_identifiers(*ast_);
    // TODO: infer types
    
    for (Expression* expression: ast_->unparsed_termed_expressions) {
        select_expression(expression);
        parse_stack_ = {};
        previous_operator_precedence_ = MAX_OPERATOR_PRECEDENCE;
        parse_termed_expression();
    }
}

void ParserLayer2::select_expression(Expression* expression) {
    current_expression_ = expression;
    current_terms_ = std::get<AST::TermedExpressionValue>(expression->value);
    next_term_it_ = current_terms_.begin();
    
    next_term_ = (next_term_it_ != current_terms_.end()) ? 
        *next_term_it_ : nullptr;    
}

Expression* ParserLayer2::get_term() {
    current_term_ = next_term_;

    next_term_it_++;

    next_term_ = (next_term_it_ < current_terms_.end()) ? 
        *next_term_it_ : nullptr;

    return current_term_;
}

AST::Expression* ParserLayer2::peek() const {
    return next_term_;
}

void ParserLayer2::shift() {
    parse_stack_.push_back(get_term());
}

bool ParserLayer2::at_expression_end() const {
    return static_cast<bool>(next_term_);
}

bool ParserLayer2::parse_stack_reduced() const {
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

Expression* ParserLayer2::parse_termed_expression() {
    if (at_expression_end()) {
        log_error(current_expression_->location, "layer2 tried to parse an empty expresison");
        ast_->declare_invalid();
        return current_expression_;
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
            log_error(current_expression_->location, "unexpected tie in termed expression");
            ast_->declare_invalid();
            break;

        default:
            assert(false && "unhandled expressiontype in ParserLayer2::parse_termed_expression");
    }

    assert(!parse_stack_.empty() && "parse_termed_expression didn't put anything on the parse_stack");

    if (at_expression_end()) {
        if (!parse_stack_reduced()) {
            log_error(current_expression_->location, 
                "parse_termed_expression failed to reduce the stack");
            ast_->declare_invalid();
            return current_expression_;
        }
        return parse_stack_.back();
    }
    
    assert(false && "parse_termed_expression didn't parse the whole expression");
    return current_expression_;
}

void ParserLayer2::parse_tied_operator() {
    log_error(current_expression_->location, "parse_tied_operator not implemented");
    ast_->declare_invalid();
}


// ----- STATE FUNCTIONS -----

void ParserLayer2::initial_identifier_state() {
    shift();
    if (at_expression_end()) {
        return;
    }

}

void ParserLayer2::initial_value_state() {
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

void ParserLayer2::initial_operator_state() {
    shift();
    if (at_expression_end()) {
        // might be partial application or postfix unary
        return;
    }

    // check arity
}

// maybe dumb?
void ParserLayer2::pre_binary_operator_state() {
    shift();
    if (at_expression_end()) {
        return;
    }
    // check type
    // compare precedence
}

void ParserLayer2::post_binary_operator_state() {
    shift();
    if (at_expression_end()) {
        return;
    }


}

void ParserLayer2::arg_list_state() {
    shift();
    if (at_expression_end()) {
        return;
    }
}

void ParserLayer2::unary_operators_state() {
    shift();
    if (at_expression_end()) {
        return;
    }
}
