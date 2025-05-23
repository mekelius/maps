/**
 * Recursive descent parser for maps programming language
 * 
 * The convention here (unlike in the lexer) is that every production rule must move the buffer
 * beyond the tokens it consumed.
 * 
 * Same if a token is rejected. 
 */
#include "../parser_layer1.hh"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <sstream>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/pragma.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/types/type.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/operator.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/ast_store.hh"

#include "mapsc/parser/token.hh"
#include "mapsc/procedures/simplify.hh"

using std::optional, std::nullopt, std::make_unique;


namespace Maps {

using Log = LogInContext<LogContext::layer1>;

// ----------- EXPRESSIONS ----------

Expression* ParserLayer1::parse_expression() {
    auto location = current_token().location;

    switch (current_token().token_type) {
        case TokenType::eof: {
            Expression* expression = Expression::valueless(*ast_store_,
                ExpressionType::user_error, location);
            return expression;
        }

        case TokenType::identifier: {
            Token next_token = peek();

            if (is_statement_separator(next_token))
                return handle_identifier();

            if (is_access_operator(next_token))
                return parse_termed_expression();

            switch (next_token.token_type) {
                case TokenType::identifier:
                case TokenType::operator_t:
                case TokenType::number:
                case TokenType::string_literal:
                case TokenType::tie:
                    return parse_termed_expression();    

                case TokenType::eof:
                case TokenType::semicolon:
                case TokenType::indent_block_end:
                    return handle_identifier();

                default:
                    return handle_identifier();
            }
        }

        case TokenType::lambda:
            return parse_lambda_expression();
        case TokenType::question_mark:
            return parse_ternary_expression();

        case TokenType::type_identifier: 
        case TokenType::arrow_operator:
        case TokenType::operator_t:
            return parse_termed_expression();

        case TokenType::number: 
            if (is_term_token(peek()))
                return parse_termed_expression();
            return handle_numeric_literal();

        case TokenType::string_literal: 
            if (is_term_token(peek()))
                return parse_termed_expression();
            return handle_string_literal();
            
        // these guys are not LL(1), so we need to always assume they are termed
        case TokenType::parenthesis_open:
        case TokenType::bracket_open:
        case TokenType::curly_brace_open:
            return parse_termed_expression();

        case TokenType::indent_block_start: {
            unsigned int starting_indent_level = indent_level_;
            get_token(); // eat the starting indent
            Expression* expression = parse_expression();
            
            if (current_token().token_type != TokenType::indent_block_end)
                return fail_expression("Mismatched indents!", current_token().location);

            get_token(); // eat the ending indent
            assert(indent_level_ <= starting_indent_level && "Mismatched indents");
            return expression;
        }

        case TokenType::reserved_word:
            if (current_token().string_value() != "let")
                assert(false && "not implemented");
            assert(false && "not implemented");            
            
        default:
            return fail_expression(
                "unexpected " + current_token().get_string() + ", at the start of an expression",
                current_token().location);
    }
}

Expression* ParserLayer1::parse_access_expression() {
    assert(false && "not implemented");
}

Expression* ParserLayer1::parse_ternary_expression() {
    assert(false && "not implemented");
}

Expression* ParserLayer1::parse_lambda_expression() {
    auto location = current_token().location;
    get_token(); // eat the '\'

    RT_Scope* lambda_scope = ast_store_->allocate_scope(RT_Scope{/*parse_scope_*/});
    auto parameter_list = parse_lambda_parameters(lambda_scope);

    if (!parameter_list)
        return fail_expression("parsing lambda parameter list failed", location);

    if (current_token().token_type != TokenType::arrow_operator)
        return fail_expression("Undexpected " + current_token().get_string() + 
            " in lambda expression, expected a \"->\" or \"=>\" ", current_token().location);

    bool is_pure = current_token().string_value() == "->";
    get_token();

    DefinitionBody body = parse_definition_body();

    return Expression::lambda(*compilation_state_, {*parameter_list, lambda_scope, body}, is_pure, 
        location);
}

optional<ParameterList> ParserLayer1::parse_lambda_parameters(RT_Scope* lambda_scope) {
    ParameterList parameter_list{};

    while (true) {
        switch (current_token().token_type) {
            case TokenType::arrow_operator:
                return parameter_list;

            case TokenType::eof:
                return fail_optional(
                    "Unexpected eof in lambda parameter list", current_token().location);

            case TokenType::identifier: {
                auto name = current_token().string_value();
                auto location = current_token().location;

                auto parameter = RT_Definition::parameter(*ast_store_, name, &Hole, location);

                // check if the string is already bound, in which case we exit
                if (!lambda_scope->create_identifier(name, parameter))
                    return fail_optional(
                        "Duplicate parameter name " + name + " in lambda parameter list", location);

                parameter_list.push_back(parameter);
                get_token();
                continue;
            }

            case TokenType::bracket_open:
            case TokenType::type_identifier: {
                auto location = current_token().location;

                auto type = parse_parameter_type_declaration();
                
                if (!type)
                    return fail_optional(
                        "Parsing lambda parameter list failed, incorrect type declaration", location);

                if (current_token().token_type != TokenType::identifier) {
                    assert(false && "Something unexpected happened, parse_parameter_type_declaration returned without an identifier as the current token");
                    return fail_optional(("Something unexpected happened, parse_parameter_type_declaration returned without an identifier as the current token"), 
                        current_token().location, true);
                }

                auto name = current_token().string_value();
                auto parameter = RT_Definition::parameter(*ast_store_, name, *type, location);

                // check if the string is already bound, in which case we exit
                if (!lambda_scope->create_identifier(name, parameter))
                    return fail_optional(
                        "Duplicate parameter name " + name + " in lambda parameter list", location);
                get_token();
                continue;
            }

            case TokenType::underscore:
                parameter_list.push_back(RT_Definition::discarded_parameter(*ast_store_, &Hole, 
                    current_token().location));
                get_token();
                continue;

            default:
                return fail_optional(
                    "Unexpected " + current_token().get_string() + " in lambda parameter list",
                    current_token().location);
        }
    }
}

std::optional<const Type*> ParserLayer1::parse_parameter_type_declaration() {
    auto location = current_token().location;

    switch (current_token().token_type) {
        case TokenType::type_identifier: {
            switch (peek().token_type) {
                case TokenType::identifier: {
                    auto type = compilation_state_->types_->get(current_token().string_value());
                    get_token();
                    return type;
                }
                case TokenType::arrow_operator:
                    assert(false && "not implemented");
                    get_token();

                case TokenType::eof:
                    fail("Unexpected eof in parameter type declaration", current_token().location);
                    return nullopt;

                default:
                    fail("Unexpected " + current_token().get_string() + 
                        " in parameter type declaration, expected an identifier or an arrow", location);
            }
        }

        case TokenType::indent_block_start:
        case TokenType::bracket_open:
        case TokenType::parenthesis_open:
        case TokenType::colon:
            assert(false && "not implemented");

        default:
            fail("Unexpected " + current_token().get_string() + 
                " in parameter type declaration, expected a type expression", location);
            return nullopt;
    }

}

Expression* ParserLayer1::parse_binding_type_declaration() {
    assert(false && "not implemented");

    // just go to termed expression
}

// expects to be called with the opening parenthese as the current_token_
Expression* ParserLayer1::parse_parenthesized_expression() {
    get_token(); // eat '('

    if (current_token().token_type == TokenType::parenthesis_close) {
        auto location = current_token().location;
        get_token();
        return fail_expression("Empty parentheses in an expression", location);
    }

    Expression* expression = parse_expression();
    if (current_token().token_type != TokenType::parenthesis_close)
        return fail_expression("Mismatched parentheses", current_token().location);

    get_token(); // eat ')'
    return expression;
}

Expression* ParserLayer1::parse_mapping_literal() {
    assert(false && "not implemented");
}

} // namespace Maps
