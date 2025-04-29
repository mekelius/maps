#ifndef __LEXER_HH
#define __LEXER_HH

#include <istream>
#include <memory>
#include <array>
#include <vector>

#include "token.hh"

class Lexer {
public:
    Lexer(std::istream* source_is, SourceID source_id = DEFAULT_SOURCE_ID);

    // extracts the next token from the stream
    Token get_token();

private:
    char read_char();
    char peek_char();
    // creates a token filled with the correct line and col info
    Token create_token(TokenType token_type);
    Token create_token(TokenType token_type, const std::string& value);
  
    // production rules
    Token get_token_();
    Token read_identifier();
    Token read_operator();
    Token read_numeric_literal();
    Token read_string_literal();
    Token read_tie_();
    Token read_linebreak();
    Token read_pragma();

    Token collapsed_semicolon_token();
    void read_and_ignore_comment();

    char current_char_ = '\00'; // this null will be read and discarded during the constructor 
    bool tie_possible_ = false; // ties mark a lack of whitespace between operators, values and identifiers
    unsigned int indents_to_close_ = 0; // if there's indents to close, close one instead of reading further
    unsigned int current_line_ = 1;
    unsigned int current_col_ = 0;
    unsigned int current_token_start_line_ = 0;
    unsigned int current_token_start_col_ = 0;

    std::stringbuf buffer_ = {};
    Token prev_token_ = Token{TokenType::dummy, {0,0}}; // a bit of a hack to keep the tokens in sync with the parser
    std::vector<unsigned int> indent_stack_ = {0};
    
    SourceID source_id_;
    std::istream* source_is_;
};

#endif