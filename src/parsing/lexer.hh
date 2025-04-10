#ifndef __LEXER_HH
#define __LEXER_HH

#include <istream>
#include <memory>
#include <array>
#include <sstream>
#include <vector>

// TODO: update the compiler and make these constexpr
const std::string OPERATOR_GLYPHS = "+-*/^=!?:|<>.$&€£@¬§¤";
bool is_operator_glyph(char);
bool is_parenthese(char);

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

class Token {
  public:
    TokenType type = TokenType::unknown;

    unsigned int line;
    unsigned int col;

    std::string value = "";
    int int_value = 0;

    std::string get_location() const;
    std::string get_str() const;
    std::string get_str(bool stream_format) const;
};

bool is_statement_separator(Token token);
bool is_block_starter(Token token);
bool is_assignment_operator(Token token);
bool is_access_operator(Token token);
bool is_term_token(Token token);

class StreamingLexer {
  public:
    StreamingLexer(std::istream* source_is, std::ostream* tokens_ostream = nullptr);

    void set_tokens_ostream(std::ostream* tokens_ostream);
    unsigned int get_current_line();
    
    // extracts the next token from the stream
    Token get_token();

  private:
  
    char read_char();
    // creates a token filled with the correct line and col info
    Token create_token_(TokenType type);
    Token create_token_(TokenType type, const std::string& value);
    Token create_token_(TokenType type, int value);
  
    // production rules
    Token get_token_();
    Token read_operator_();
    Token read_identifier_();
    Token read_numeric_literal_();
    Token read_string_literal_();
    Token read_tie_();
    Token read_linebreak_();
    Token read_pragma();

    Token collapsed_semicolon_token_();
    void read_and_ignore_comment_();

    char current_char_ = '\00'; // this null will be read and discarded during the constructor 
    bool tie_possible_ = false; // ties mark a lack of whitespace between operators, values and identifiers
    unsigned int indents_to_close_ = 0; // if there's indents to close, close one instead of reading further
    unsigned int current_line_ = 1;
    unsigned int current_col_ = 0;
    unsigned int current_token_start_line_ = 0;
    unsigned int current_token_start_col_ = 0;

    std::stringbuf buffer_ = {};
    TokenType prev_token_type_ = TokenType::bof;
    std::string prev_token_str_ = ""; // a bit of a hack to keep the tokens in sync with the parser
    std::vector<unsigned int> indent_stack_ = {0};
    
    std::istream* source_is_;
    std::ostream* tokens_os_;
};

std::ostream& operator<<(std::ostream& os, Token token);

#endif