#include "implementation.hh"

#include <cassert>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"
#include "mapsc/ast/identifier.hh"
#include "mapsc/ast/layer2_expression.hh"
#include "mapsc/ast/misc_expression.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/expression_properties.hh"

#include "mapsc/parser/token.hh"

using std::optional, std::nullopt, std::make_unique;


namespace Maps {

using Log = LogInContext<LogContext::layer1>;


// expects to be called with the first term as current
// tied expression = no whitespace
Expression* ParserLayer1::parse_termed_expression(bool in_tied_expression) {
    auto context = current_context();

    if (!context)
        return fail_expression("Termed expressions require a context", current_token().location, true);

    Expression* expression = create_layer2_expression(*ast_store_, {}, *context, current_token().location);

    log(
        in_tied_expression ? "start parsing tied expression" : "start parsing termed expression", 
        LogLevel::debug_extra);

    expression->terms().push_back(parse_term(in_tied_expression));
    
    if (!is_allowed_in_type_declaration(*expression->terms().back()))
        expression->mark_not_type_declaration();

    bool done = false;
    while (!done) {
        // in tied expressions, every term must be followed by a tie
        // if not, we are done
        if(in_tied_expression) {
            if (current_token().token_type != TokenType::tie || current_token().token_type == TokenType::eof) {
                done = true;
                break;
            } else {
                get_token(); // eat the tie
            }
        }

        switch (current_token().token_type) {
            // ties create a new sub-expression, unless we are already in one
            case TokenType::tie:
                assert(false && "somehow a tie leaked into the termed expression loop");
                get_token();
                break;

            case TokenType::eof:
            case TokenType::indent_block_end:
            case TokenType::semicolon:
            case TokenType::parenthesis_close:
            case TokenType::bracket_close:
            case TokenType::curly_brace_close:
                done = true;
                break;
                
            case TokenType::colon: {
                assert(!in_tied_expression && "colon shouldn't be tieable");

                // colon has to add parenthesis around left side as well
                if (expression->terms().size() > 1) {
                    Expression* lhs = create_layer2_expression(*ast_store_, {}, *context, expression->location);
                    *lhs = *expression;
                    expression->terms() = {close_termed_expression(lhs)};
                    std::get<TermedExpressionValue>(expression->value).is_type_declaration = 
                        DeferredBool::maybe_;
                }

                // eat the ":" and any following ones as they wouldn't do anything
                while (current_token().token_type == TokenType::colon) get_token(); 
                
                expression->terms().push_back(parse_termed_expression(false));

                if (!is_allowed_in_type_declaration(*expression->terms().back()))
                    expression->mark_not_type_declaration();
                break;
            }

            case TokenType::lambda:
            case TokenType::parenthesis_open:
            case TokenType::bracket_open:
            case TokenType::curly_brace_open:
                expression->terms().push_back(parse_term(in_tied_expression));
                if (!is_allowed_in_type_declaration(*expression->terms().back()))
                    expression->mark_not_type_declaration();
                break;

            case TokenType::indent_block_start:
                if (is_expression_ender(peek())) {
                    done = true;
                    break;
                }
                // intentional fall-through
            case TokenType::string_literal:
            case TokenType::number:
            case TokenType::identifier:
            case TokenType::operator_t:
            case TokenType::type_identifier:
            case TokenType::arrow_operator:
                expression->terms().push_back(parse_term(in_tied_expression));
                if (!is_allowed_in_type_declaration(*expression->terms().back()))
                    expression->mark_not_type_declaration();
                break;

            default:
                return fail_expression(
                    "Unexpected: " + current_token().get_string() + ", in termed expression",
                    current_token().location);
        }
    }

    log("Finished parsing termed expression from " + expression->location.to_string(), 
        LogLevel::debug_extra);
    return close_termed_expression(expression);
}

Expression* ParserLayer1::close_termed_expression(Expression* expression) {
    assert(expression->expression_type == ExpressionType::layer2_expression &&
        "close_termed_expression called with not a termed expression");

    // unwrap redundant parentheses
    if (expression->terms().size() == 1) {
        auto term = expression->terms().at(0);
        ast_store_->delete_expression(expression);

        log("removed \"parentheses\" from " + expression->location.to_string(), 
            LogLevel::debug_extra);
        return term;
    }

    // handle possible binding type declaration
    if (expression->terms().size() == 2) {
        Expression* lhs = expression->terms().at(0);
        Expression* rhs = expression->terms().at(1);

        if ( rhs->is_type_declaration() != DeferredBool::true_ &&
             lhs->is_type_declaration() != DeferredBool::false_
        ) result_.possible_binding_type_declarations.push_back(lhs);
    }

    result_.unparsed_termed_expressions.push_back(expression);

    return expression;
}

// Expects not to be called if the current token is not parseable into a term
Expression* ParserLayer1::parse_term(bool is_tied) {
    // if we see a tie, go down into a tied expression if not already in one
    if (peek().token_type == TokenType::tie && !is_tied)
        return parse_termed_expression(true);

    switch (current_token().token_type) {
        case TokenType::identifier:
            // TODO: revamp access expressions
            return handle_identifier();
        case TokenType::string_literal:
            return handle_string_literal();
        case TokenType::number:
            return handle_numeric_literal();

        case TokenType::colon:
            fail("unhandled token type: " + current_token().get_string() + 
                ", reached ParserLayer1::parse_term", current_token().location);
            assert(false && "colons should be handled by parse_termed expression");
            return create_user_error(*ast_store_, current_token().location);

        case TokenType::parenthesis_open: 
            return parse_parenthesized_expression();

        case TokenType::bracket_open:
        case TokenType::curly_brace_open:
            return parse_mapping_literal();
        
        case TokenType::indent_block_start:
            return parse_expression();
            
        case TokenType::operator_t: {
            // handle minus sign as a special case
            if (current_token().string_value() == "-") {
                auto location = current_token().location;
                get_token();
                return create_minus_sign(*ast_store_, location);
            }

            Expression* expression = create_operator_identifier(*ast_store_, parse_scope_,
                current_token().string_value(), current_token().location);
            result_.unresolved_identifiers.push_back(expression);

            get_token();
            return expression;
        }

        case TokenType::type_identifier:
            return handle_type_identifier();

        case TokenType::arrow_operator:
            assert(false && "not implemented");
            // return Expression::type_operator_reference(*ast_store_,
            //     current_token().string_value(), current_token().location);

        case TokenType::question_mark:
            return parse_ternary_expression();

        case TokenType::lambda:
            return parse_lambda_expression();

        default:
            return fail_expression("In parse_term: unhandled token type: " + current_token().get_string(), 
                current_token().location, true);
    }
}

} // namespace Maps
