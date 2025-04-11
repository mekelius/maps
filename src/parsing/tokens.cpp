#include <cassert>

#include "tokens.hh"
#include "../logging.hh"

Token::Token(TokenType token_type, SourceLocation location, Value value)
:token_type(token_type), location(location), value(value) {
    switch (token_type) {
        case TokenType::identifier: 
        case TokenType::operator_t:
        case TokenType::number: 
        case TokenType::string_literal:
        case TokenType::reserved_word:
        case TokenType::pragma:
            assert((std::holds_alternative<std::string>(value) || std::holds_alternative<std::monostate>(value))
                && "Token created with wrong value type");
            if (std::holds_alternative<std::monostate>(value))
                value = "";
            break;

        case TokenType::indent_error_fatal:
        case TokenType::indent_block_start: 
        case TokenType::indent_block_end: 
        case TokenType::eof:
        case TokenType::curly_brace_open: 
        case TokenType::curly_brace_close:
        case TokenType::parenthesis_open: 
        case TokenType::parenthesis_close:
        case TokenType::bracket_open: 
        case TokenType::bracket_close:
        case TokenType::semicolon: 
        case TokenType::comma: 
        case TokenType::lambda:
        case TokenType::tie:
        case TokenType::dummy:
            assert(std::holds_alternative<std::monostate>(value) && "Token created with wrong value type");
            break;

        default:
            assert(false && "Unhandled token type in Token constructor");
    }
}

std::string Token::get_string() const {
    switch (token_type) {
        case TokenType::eof:
            return "EOF";
        case TokenType::dummy:
            return "DUMMY TOKEN";

        case TokenType::identifier:
            return "identifier: " + string_value();
        case TokenType::operator_t:
            return "operator: " + string_value();
    
        case TokenType::number:
            return "numeric literal: " + string_value();
        case TokenType::string_literal:
            return "string literal \"" + string_value() + "\"";

        case TokenType::reserved_word:
            return "reserved word " + string_value();
        
        case TokenType::indent_block_start:
            return "indent block start";
        case TokenType::indent_block_end:
            return "indent block end";
        case TokenType::indent_error_fatal:
            return "indent error fatal";

        case TokenType::curly_brace_open:
            return "{";
        case TokenType::curly_brace_close:
            return "}";
        case TokenType::parenthesis_open:
            return "(";
        case TokenType::parenthesis_close:
            return ")";
        case TokenType::bracket_open:
            return "[";
        case TokenType::bracket_close:
            return "]";
        case TokenType::semicolon:
            return ";";
        case TokenType::comma:
            return ",";
        case TokenType::lambda:
            return "\\";

        case TokenType::tie:
            return "tie";

        case TokenType::pragma:
            return "pragma: " + string_value();

        default:
            assert(false && "tokentype is lacking a string representation");
    }
}

bool is_assignment_operator(const Token& token) {
    if (token.token_type != TokenType::operator_t)
        return false;       
    
    return (
        token.string_value() == "="    //||
        // token.value == "+="     ||
        // token.value == "-="     ||
        // token.value == "++"     ||
        // token.value == "--"     ||
        // token.value == "?="     ||
    );
}

bool is_statement_separator(const Token& token) {
    switch (token.token_type) {
        case TokenType::semicolon:
        case TokenType::eof:
        case TokenType::indent_block_end:
        case TokenType::indent_error_fatal:
        case TokenType::curly_brace_close:
        case TokenType::bracket_close:
        case TokenType::parenthesis_close:
            return true;
                
        default: 
            return false;
    }
}

bool is_access_operator(const Token& token) {
    switch (token.token_type) {
        case TokenType::operator_t:
            if (token.string_value() != "::" && token.string_value() != ".")
                return false;

        case TokenType::parenthesis_open:
        case TokenType::curly_brace_open:
        case TokenType::bracket_open:
            return true;
        
        default:
            return false;
    }
}

bool is_block_starter(const Token& token) {
    switch (token.token_type) {
        case TokenType::indent_block_start:        
        case TokenType::curly_brace_open:
            return true;
        default:
            return false;
    }
}

bool is_tieable_token(const Token& token) {
    return (
        token.token_type == TokenType::operator_t     ||
        token.token_type == TokenType::identifier     ||
        token.token_type == TokenType::number         ||
        token.token_type == TokenType::string_literal
    );
}

bool is_term_token(const Token& token) {
    switch (token.token_type) {
        case TokenType::string_literal:
        case TokenType::number:
        case TokenType::parenthesis_open:
        case TokenType::bracket_open:
        case TokenType::curly_brace_open:
        case TokenType::identifier:
        case TokenType::indent_block_start:
        case TokenType::lambda:
        case TokenType::operator_t:
        case TokenType::tie:
        case TokenType::reserved_word:
            return true;

        default:
            return false;
    }
}

const Token Token::dummy_token {TokenType::dummy, {0,0}};