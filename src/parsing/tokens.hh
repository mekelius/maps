#ifndef __TOKENS_HH
#define __TOKENS_HH

#include <sstream>

#include "../logging.hh"

enum class TokenType: int {
    bof, eof,
    identifier, operator_t,
    number, string_literal,
    reserved_word,
    indent_block_start, indent_block_end, indent_error_fatal,
    curly_brace_open, curly_brace_close,
    parenthesis_open, parenthesis_close,
    bracket_open, bracket_close,
    semicolon, comma, lambda,
    tie,
    pragma,
    unknown,
};

class Token {
  public:
    TokenType type = TokenType::unknown;

    SourceLocation location;

    std::string value = "";
    int int_value = 0;

    std::string get_str() const;
    std::string get_str(bool stream_format) const;
};

std::ostream& operator<<(std::ostream& os, Token token);

bool is_tieable_token_type(TokenType token_type);  
bool is_statement_separator(Token token);
bool is_block_starter(Token token);
bool is_assignment_operator(Token token);
bool is_access_operator(Token token);
bool is_term_token(Token token);

#endif