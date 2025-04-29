#ifndef __TOKENS_HH
#define __TOKENS_HH

#include <sstream>
#include <variant>

#include "../logging.hh"

enum class TokenType: int {
    eof,
    identifier, type_identifier, 
    operator_t, 
    arrow_operator, // -> and =>
    number, string_literal,
    reserved_word,
    indent_block_start, indent_block_end, indent_error_fatal,
    curly_brace_open, curly_brace_close,
    parenthesis_open, parenthesis_close,
    bracket_open, bracket_close,
    semicolon, colon, double_colon, comma, lambda,
    tie,
    pragma,
    dummy,  // dummy token used to initialize token values
    unknown // unhandled, means a bug
};

struct Token {
    Token(TokenType token_type, const std::string& value, SourceLocation location);
    Token(TokenType token_type, SourceLocation location);

    TokenType token_type;
    std::string value;
    SourceLocation location;

    // the raw string value
    // getter kept to save on changes
    const std::string& string_value() const {
        return value;
    }

    // a formatted representation of the token
    std::string get_string() const;

    static const Token dummy_token;
};

std::ostream& operator<<(std::ostream& os, const Token& token);

bool is_assignment_operator(const Token& token);
bool is_statement_separator(const Token& token);
bool is_access_operator(const Token& token);
bool is_block_starter(const Token& token);
bool is_tieable_token(const Token& token);  
bool is_term_token(const Token& token);

#endif