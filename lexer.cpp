#include "lexer.hh"

#include <cctype>
#include <sstream>
#include <string>
#include <algorithm>
#include <cassert>

const Token EOF_TOKEN = { TokenType::eof };
const Token UNKNOWN_TOKEN = { TokenType::unknown };
const Token SEMICOLON_TOKEN = { TokenType::semicolon };
const Token INDENT_START_TOKEN = { TokenType::indent_block_start };

bool is_operator_glyph(char glyph) {
    return OPERATOR_GLYPHS.find(glyph) != std::string::npos;
}

bool is_reserved_word(const std::string& word) {
    return std::find(RESERVED_WORDS.begin(), RESERVED_WORDS.end(), word) != RESERVED_WORDS.end();
}

// ----- StreamingLexer -----
StreamingLexer::StreamingLexer(std::istream* source_is): 
source_is_(source_is) {
    read_char();
}

Token StreamingLexer::get_token() {
    Token token = get_token_();
    prev_token_type_ = token.type;
    return token;
}

char StreamingLexer::read_char() {
    source_is_->get(current_char_);
    return current_char_;
}

Token StreamingLexer::read_operator_() {
    // cannot reset the buffer, since there might be an initial '/' there
    while (is_operator_glyph(current_char_)) {
        buffer_.sputc(current_char_);
        read_char();
        if (source_is_->eof())
            return EOF_TOKEN;
    }
    return {
        TokenType::operator_t,
        buffer_.str()
    };
}

Token StreamingLexer::read_identifier_() {
    buffer_ = {};
    buffer_.sputc(current_char_);
    read_char();
    
    while ((std::isalnum(current_char_) || current_char_ == '_') && !source_is_->eof()) {
        buffer_.sputc(current_char_);
        read_char();
    }

    std::string value = buffer_.str();
    if (is_reserved_word(value)) {
        return {
            TokenType::reserved_word,
            value
        };
    }
        
    return {
        TokenType::identifier,
        value
    };
}

Token StreamingLexer::read_string_literal_() {
    buffer_ = {};

    while (read_char() != '\"' && !source_is_->eof()) {
        buffer_.sputc(current_char_);
    }

    read_char(); // eat the closing "
    return {
        TokenType::string_literal,
        buffer_.str()
    };
}

Token StreamingLexer::read_numeric_literal_() {
    do {
        buffer_.sputc(current_char_);
        read_char();
    } while ((std::iswdigit(current_char_) || current_char_ == '.') && !source_is_->eof());

    return {
        TokenType::number,
        buffer_.str()
    };
}

// whitespace has to be read, since it's sometimes significant
// try to omit it when possible
Token StreamingLexer::read_whitespace_() {
    while (current_char_ == ' ') {
        read_char();
        if (source_is_->eof())
            return EOF_TOKEN;
    }

    // whitespace can only affect semantics if the previous token is an operator or an identifier
    // indentation is handled by read_linebreak_
    switch (prev_token_type_) {
        case TokenType::operator_t:
        case TokenType::identifier:
            return {
                TokenType::whitespace
            };
        default:
            return get_token_();
    }
}

Token StreamingLexer::read_linebreak_() {
    unsigned int current_indent = indent_stack_.back();
    unsigned int next_line_indent = 0;
    current_line_++;

    // eat empty lines
    while (read_char() == '\n' && !source_is_->eof())
        current_line_++;

    // determine next line indent
    // TODO: deal with tab characters
    while (current_char_ == ' ' && !source_is_->eof()) {
        next_line_indent++;
        read_char();
    }

    // in case of another newline just start again
    if (current_char_ == '\n' && !source_is_->eof())
        return read_linebreak_();

    // deal with possible comments
    if (current_char_ == '/') {
        char peeked_char = source_is_->peek();
        // it's a comment
        if (peeked_char == '/' || peeked_char == '*') {
            read_char(); // read_and_ignore comment expects us to reat the initial '/'
            read_and_ignore_comment_();
            // !!!: multi-line comments will mess with indentation here
            return read_linebreak_();
        }
    }

    // if the indent didn't change, just insert a semicolon
    if (next_line_indent == current_indent) 
        return collapsed_semicolon_token_();

    // if the indent increased, start a new level
    if (next_line_indent > current_indent) {
        indent_stack_.push_back(next_line_indent);
        return INDENT_START_TOKEN;
    }

    // if the indent decreased, close the previous indents we passed
    unsigned int levels_closed = 0;
    while (next_line_indent < indent_stack_.back()) {
        levels_closed++;
        indent_stack_.pop_back();
    }
    
    // if the indent went down to a level that was not used before, the code is invalid
    if (next_line_indent != indent_stack_.back()) {
        return {
            TokenType::indent_error_fatal,
            "Mismatched indent",
            static_cast<int>(current_line_)
        };
    }

    return {
        TokenType::indent_block_end,
        "",
        static_cast<int>(levels_closed)
    };    
}

// Reduce redundant semicolons
Token StreamingLexer::collapsed_semicolon_token_() {
    switch (prev_token_type_) {
        case TokenType::indent_block_start:
        case TokenType::indent_block_end:
        case TokenType::semicolon:
            return get_token_();
        
        default:
            return SEMICOLON_TOKEN;
    }
}

// when calling this, the caller must eat the first '/' of the comment
// the caller is also responsible for checking that current_char_ is either '/' or '*'
void StreamingLexer::read_and_ignore_comment_() {
    // single-line comment
    if (current_char_ == '/') {
        while (read_char() != '\n') {
            if (source_is_->eof())
                return;
        }
        return;
    }

    assert(current_char_ == '*');
    
    // multi-line comment
    // NOTE: multi-line comments may mess with indentation, but can't be bothered to think about that atm
    bool found_closing = false;
    bool prev_was_asterisk = false;
        
    while (!found_closing) {
        read_char();

        if (source_is_->eof())
            return;

        if (prev_was_asterisk && current_char_ == '/')
            return;

        prev_was_asterisk = current_char_ == '*';
    }

    return;
}

Token StreamingLexer::get_token_() {
    buffer_ = {};

    if (source_is_->eof())
        return EOF_TOKEN;

    switch (current_char_) {
        case EOF:
            return EOF_TOKEN;

        // handle string literals
        // TODO: handle '
        // TODO: string interpolation
        case '\"':
            return read_string_literal_();

        // handle whitespace
        case ' ':
            return read_whitespace_();
    
        case '/':
            read_char();

            // handle operator case
            if (current_char_ != '/' && current_char_ != '*') {
                buffer_.sputc('/');
                return read_operator_();
            }

            read_and_ignore_comment_();
            return get_token_();

            // handle operator
            return read_operator_();

        // handle single-character tokens
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case ':':
            {
                std::string value{1, current_char_};
                read_char();
                return { TokenType::char_token, value };
            }
        
        case '\n':
            return read_linebreak_();

        case ';':
            while(read_char() == ';' && !source_is_->eof());
            return collapsed_semicolon_token_();


        default:
            // handle identifiers
            if (std::isalpha(current_char_) || current_char_ == '_')
                return read_identifier_();

            // handle numerics
            // TODO: how to support spaces in numerics?
            // TODO: hex and oct
            if (std::iswdigit(current_char_))
                return read_numeric_literal_();

            // handle operators
            if (is_operator_glyph(current_char_))
                return read_operator_();

            // unknown token
            read_char();
            return UNKNOWN_TOKEN;
    }
}