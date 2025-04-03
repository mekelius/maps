#ifndef __LEXER_HH
#define __LEXER_HH

#include <istream>
#include <memory>
#include <array>
#include <sstream>
#include <vector>

#include "config.hh"

// TODO: update the compiler and make these constexpr
const std::string OPERATOR_GLYPHS = "+-*/^=!?:|<>.$&€£@¬§¤";
bool is_operator_glyph(char);
bool is_parenthese(char);


enum class TokenType: int {
    bof,
    eof,
    identifier,
    number,
    operator_t,
    string_literal,
    reserved_word,
    unknown,
    whitespace,
    indent_block_start,
    indent_block_end,
    indent_error_fatal,
    semicolon,
    char_token // includes single-character tokens that can't be used in operators like: (){}[]:;
};

// OPT: roll these into a single enum
const std::array<std::string, 36> RESERVED_WORDS = {
    "if", "else",
    "for", "while", "do",
    "return",
    "match", "case", "switch", "default",
    "from",
    "let", "const", "var", "type", "class",
    "has", "in", "of",                          // has could be a builtin
    "with",
    "not", "and", "or", "xor", "nor", "nand",   // these could be builtins 
    "is", "typeof", "derives", "from",
    "extern",
    "async", "await", "maybe", "value", "fail", // might be builtins
};
bool is_reserved_word(const std::string& word);

struct Token {
    TokenType type = TokenType::unknown;
    std::string value = "";
    int int_value = 0;
};

class StreamingLexer {
  public:
    StreamingLexer(std::istream* source_is, std::ostream* tokens_ostream = nullptr);

    void set_tokens_ostream(std::ostream* tokens_ostream);
    
    // extracts the next token from the stream
    Token get_token();

  private:
    Token get_token_();

    char read_char();

    Token read_operator_();
    Token read_identifier_();
    Token read_numeric_literal_();
    Token read_string_literal_();
    Token read_whitespace_();
    Token read_linebreak_();

    Token collapsed_semicolon_token_();
    void read_and_ignore_comment_();

    char current_char_;
    unsigned int current_line_ = 1;
    std::stringbuf buffer_ = {};
    TokenType prev_token_type_ = TokenType::bof;
    std::istream* source_is_;
    std::ostream* tokens_os_;
    std::vector<unsigned int> indent_stack_ = {0};
};

std::ostream& operator<<(std::ostream& os, Token token);

#endif