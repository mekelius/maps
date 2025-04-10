#include <cassert>

#include "tokens.hh"

#include "../logging.hh"

bool is_tieable_token_type(TokenType token_type) {
    return (
        token_type == TokenType::operator_t     ||
        token_type == TokenType::identifier     ||
        token_type == TokenType::number         ||
        token_type == TokenType::string_literal
    );
}

std::string Token::get_location() const {
    return std::to_string(line) + ":" + std::to_string(col);
}

// TODO: refactor
std::string Token::get_str(bool stream_format) const {
    if (!stream_format) return get_str();
    return get_location() + Logging::line_col_padding(get_location().size()) + "token: " + get_str();
}

std::string Token::get_str() const {
    switch (type) {
        case TokenType::eof:
            return "EOF";
        case TokenType::bof:
            return "BOF";

        case TokenType::identifier:
            return "identifier: " + value;
        case TokenType::operator_t:
            return "operator: " + value;
    
        case TokenType::number:
            return "numeric literal: " + value;
        case TokenType::string_literal:
            return "string literal \"" + value + "\"";

        case TokenType::reserved_word:
            return "reserved word " + value;
        
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
            return "pragma: " + value;

        case TokenType::unknown:
            return "unknown token";

        default:
            assert(false && "tokentype is lacking a string representation");
    }
}

std::ostream& operator<<(std::ostream& os, Token token) {
    return os << token.get_str(true);
}

bool is_statement_separator(Token token) {
    switch (token.type) {
        case TokenType::semicolon:
        case TokenType::eof:
        case TokenType::indent_block_end:
        case TokenType::indent_error_fatal:
        case TokenType::unknown:
        case TokenType::curly_brace_close:
        case TokenType::bracket_close:
        case TokenType::parenthesis_close:
            return true;
                
        default: 
            return false;
    }
}

bool is_block_starter(Token token) {
    switch (token.type) {
        case TokenType::indent_block_start:        
        case TokenType::curly_brace_open:
            return true;
        default:
            return false;
    }
}

bool is_assignment_operator(Token token) {
    if (token.type != TokenType::operator_t)
        return false;       
    
    return (
        token.value == "="      //||
        // token.value == "+="     ||
        // token.value == "-="     ||
        // token.value == "++"     ||
        // token.value == "--"     ||
        // token.value == "?="     ||
    );
}

bool is_access_operator(Token token) {
    switch (token.type) {
        case TokenType::operator_t:
            if (token.value != "::" && token.value != ".")
                return false;

        case TokenType::parenthesis_open:
        case TokenType::curly_brace_open:
        case TokenType::bracket_open:
            return true;
        
        default:
            return false;
    }
}

bool is_term_token(Token token) {
    switch (token.type) {
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