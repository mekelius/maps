#ifndef __LEXER_HH
#define __LEXER_HH

#include <istream>
#include <memory>
#include <array>

#include "ir_gen.hh"

// TODO: update the compiler and make these constexpr
const std::string OPERATOR_GLYPHS = "+-*/=!?|<>.$&^€£@¬§¤\\";
bool is_operator_glyph(char);
bool is_parenthese(char);


enum class TokenType: int {
    eof,
    identifier,
    number,
    operator_t,
    string_literal,
    reserved_word,
    unknown,
    whitespace,
    char_token // includes single-character tokens that can't be used in operators like: (){}[]:;
};

const std::array<std::string, 1> RESERVED_WORDS = {
    "let"
};
bool is_reserved_word(const std::string& word);

struct Token {
    TokenType type = TokenType::unknown;
    std::string value = "";
};

class StreamingLexer {
  public:
    StreamingLexer(std::istream* source_is);

    // extracts the next token from the stream
    Token get_token();

  private:
    char read_char();
    char current_char_;
    std::istream* source_is_;
};


// First attempt at a parser. Parses tokens directly into the llvm context
class DirectParser {
  public:
    DirectParser(StreamingLexer* lexer, IRGenHelper* ir_gen);

    void run();
  private:
    StreamingLexer* lexer_;
    IRGenHelper* ir_gen_;   
    Token current_token_;
};

#endif