#include "token.hh"

#include <cassert>

Token::Token(TokenType token_type, const std::string& value, SourceLocation location)
:token_type(token_type), value(value), location(location) {}

Token::Token(TokenType token_type, SourceLocation location)
:Token(token_type, "", location) {}

std::string Token::get_string() const {
    switch (token_type) {
        case TokenType::eof:
            return "EOF";
        case TokenType::dummy:
            return "DUMMY TOKEN";

        case TokenType::identifier:
            return "identifier: " + string_value();
        case TokenType::type_identifier:
            return "type identifier: " + string_value();
        case TokenType::operator_t:
            return "operator: " + string_value();
        case TokenType::arrow_operator:
            return "type operator: " + string_value();    
    
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
        case TokenType::colon:
            return ":";
        case TokenType::double_colon:
            return "::";
        case TokenType::comma:
            return ",";
        case TokenType::lambda:
            return "\\";

        case TokenType::tie:
            return "tie";

        case TokenType::pragma:
            return "pragma: " + string_value();

        case TokenType::unknown:
            assert(false && "unknown tokens shouldn't exist");
            return "unknown token type";
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
            return (token.string_value() == ".");

        case TokenType::parenthesis_open:
        case TokenType::curly_brace_open:
        case TokenType::bracket_open:
        case TokenType::double_colon:
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

// these can be FOLLOWED by a tie
bool is_right_tieable_token(const Token& token) {
    // ? should type identifiers be tieable?
    return (
        token.token_type == TokenType::operator_t      ||
        token.token_type == TokenType::identifier      ||
        token.token_type == TokenType::number          ||
        token.token_type == TokenType::double_colon    ||
        token.token_type == TokenType::string_literal  ||
        token.token_type == TokenType::type_identifier ||
        token.token_type == TokenType::arrow_operator
    );
}

// these are allowed to appear in termed expressions
bool is_term_token(const Token& token) {
    switch (token.token_type) {
        case TokenType::string_literal:
        case TokenType::number:
        case TokenType::parenthesis_open:
        case TokenType::bracket_open:
        case TokenType::curly_brace_open:
        case TokenType::identifier:
        case TokenType::type_identifier:
        case TokenType::indent_block_start:
        case TokenType::lambda:
        case TokenType::operator_t:
        case TokenType::arrow_operator:
        case TokenType::tie:
        case TokenType::reserved_word:
        case TokenType::colon:
        case TokenType::double_colon:
            return true;

        default:
            return false;
    }
}

const Token Token::dummy_token{TokenType::dummy, NO_SOURCE_LOCATION};