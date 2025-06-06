#ifndef __TOKENS_HH
#define __TOKENS_HH

#include <array>
#include <utility>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <map>

#include "mapsc/logging.hh"
#include "mapsc/source_location.hh"

namespace Maps {

class LogStream;

enum class TokenType: int {
    eof,
    identifier, type_identifier, 
    operator_t, 
    arrow_operator, // -> and =>
    number, string_literal,
    let, return_t, if_t, then, else_t, while_t, for_t, do_t, guard, switch_t, case_t, yield_t, 
    operator_rwt, unary, binary, prefix, postfix, 
    indent_block_start, indent_block_end, indent_error_fatal,
    curly_brace_open, curly_brace_close,
    parenthesis_open, parenthesis_close,
    bracket_open, bracket_close,
    semicolon, colon, question_mark, double_colon, comma, lambda, underscore,
    tie,
    pragma,
    dummy,  // dummy token used to initialize token values
    syntax_error
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
    LogStream::InnerStream& log_self_to(LogStream::InnerStream&) const;

    static const Token dummy_token;
};

std::optional<TokenType> lookup_reserved_word_token_type(const std::string& str);

bool is_assignment_operator(const Token& token);
bool is_statement_separator(const Token& token);
bool is_expression_ender(const Token& token);
bool is_access_operator(const Token& token);
bool is_block_starter(const Token& token);
bool is_right_tieable_token(const Token& token);  
bool is_term_token(const Token& token);
bool is_condition_ender(const Token& token);

} // namespace Maps

#endif