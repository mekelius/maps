#include "lexer.hh"

#include <cctype>
#include <sstream>
#include <string>
#include <algorithm>
#include <cassert>

#include "config.hh"

bool is_operator_glyph(char glyph) {
    return OPERATOR_GLYPHS.find(glyph) != std::string::npos;
}

// TODO: this is kinda needless and slow
bool is_reserved_word(const std::string& word) {
    return std::find(RESERVED_WORDS.begin(), RESERVED_WORDS.end(), word) != RESERVED_WORDS.end();
}

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

std::string Token::get_str(bool stream_format) const {
    if (!stream_format) return get_str();
    return get_location() + line_col_padding(get_location().size()) + "token: " + get_str();
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

// ----- Public methods -----
StreamingLexer::StreamingLexer(std::istream* source_is, std::ostream* tokens_ostream): 
source_is_(source_is),
tokens_os_(tokens_ostream) {
    read_char();
}

void StreamingLexer::set_tokens_ostream(std::ostream* tokens_ostream) {
    tokens_os_ = tokens_ostream;
}

unsigned int StreamingLexer::get_current_line() {
    return current_line_;
}

Token StreamingLexer::get_token() {
    Token token = get_token_();

    // if output stream was given, print the token there as well
    if (tokens_os_) {
        // a bit of a hack to keep the outputs in sync
        (*tokens_os_) << prev_token_str_;
        prev_token_str_ = token.get_str(true) + "\n";
    }

    tie_possible_ = is_tieable_token_type(token.type);

    prev_token_type_ = token.type;
    return token;
}

// ----- Private methods -----

// Read a character from the input stream
char StreamingLexer::read_char() {
    if (current_char_ == '\n') {
        current_col_ = 1;
        current_line_++;
    } else {
        current_col_++;
    }

    source_is_->get(current_char_);

    // just pretend like CRLF doesn't exist
    return current_char_ != '\r' ? current_char_ : read_char();
}

Token StreamingLexer::create_token_(TokenType type) {
    return Token{type, current_token_start_line_, current_token_start_col_};
}

Token StreamingLexer::create_token_(TokenType type, const std::string& value) {
    return Token{type, current_token_start_line_, current_token_start_col_, value};
}

Token StreamingLexer::create_token_(TokenType type, int value) {
    return Token{type, current_token_start_line_, current_token_start_col_, "", value};
}

// ----- PRODUCTION RULES -----

Token StreamingLexer::get_token_() {
    if (indents_to_close_ > 0) {
        indents_to_close_--;
        return create_token_(
            TokenType::indent_block_end,
            ""
        );
    }

    current_token_start_col_ = current_col_;
    current_token_start_line_ = current_line_;

    buffer_ = {};

    if (source_is_->eof())
        return create_token_(TokenType::eof);

    switch (current_char_) {
        case EOF:
            return create_token_(TokenType::eof);

        // handle string literals
        // TODO: handle '
        // TODO: string interpolation
        case '\"':
            return read_string_literal_();

        // handle whitespace
        case ' ':
            while (read_char() == ' ');
            tie_possible_ = false;
            return get_token_();

        case '#':
            return read_pragma();
    
        case '/':
            read_char();

            // handle operators that begin with '/'
            // TODO: just use peek
            if (current_char_ != '/' && current_char_ != '*') {
                buffer_.sputc('/');
                return read_operator_();
            }

            read_and_ignore_comment_();
            return get_token_();

            // handle operator
            return read_operator_();

        case '(':
            if (tie_possible_) {
                tie_possible_ = false;
                return create_token_(TokenType::tie);
            }
            read_char();
            return create_token_(TokenType::parenthesis_open);

        case '[':
            read_char();
            if (tie_possible_) {
                tie_possible_ = false;
                return create_token_(TokenType::tie);
            }
            read_char();
            return create_token_(TokenType::bracket_close);

        case '{':
            if (tie_possible_) {
                tie_possible_ = false;
                return create_token_(TokenType::tie);
            }
            read_char();
            return create_token_(TokenType::curly_brace_open);

        case ')':
            read_char();
            return create_token_(TokenType::parenthesis_close);
        case ']':
            read_char();
            return create_token_(TokenType::bracket_close);
        case '}':
            read_char();
            return create_token_(TokenType::curly_brace_close);
        case ',':
            read_char();
            return create_token_(TokenType::comma);
        case '\\':
            read_char();
            return create_token_(TokenType::lambda);
        
        case '\n':
            return read_linebreak_();

        case ';':
            while(read_char() == ';' && !source_is_->eof());
            return collapsed_semicolon_token_();

        default:
            // handle identifiers
            // TODO: handle suffixes
            if (std::isalpha(current_char_) || current_char_ == '_')
                return read_identifier_();

            // handle numerics
            // TODO: how to support spaces in numerics?
            // TODO: hex and oct
            if (std::iswdigit(current_char_))
                return read_numeric_literal_();

            // handle operators
            // TODO: move into a case expression
            if (is_operator_glyph(current_char_))
                return read_operator_();

            // unknown token
            read_char();
            return create_token_(TokenType::unknown);
    }
}

Token StreamingLexer::read_operator_() {
    if (tie_possible_) {
        tie_possible_ = false;
        return create_token_(TokenType::tie);
    }

    // cannot reset the buffer, since there might be an initial '/' there
    while (is_operator_glyph(current_char_)) {
        buffer_.sputc(current_char_);
        read_char();
        if (source_is_->eof())
            return create_token_(TokenType::eof);
    }

    return create_token_(
        TokenType::operator_t,
        buffer_.str()
    );
}

Token StreamingLexer::read_identifier_() {
    if (tie_possible_) {
        tie_possible_ = false;
        return create_token_(TokenType::tie);
    }

    buffer_ = {};
    buffer_.sputc(current_char_);
    read_char();
    
    while ((std::isalnum(current_char_) || current_char_ == '_') && !source_is_->eof()) {
        buffer_.sputc(current_char_);
        read_char();
    }

    std::string value = buffer_.str();
    if (is_reserved_word(value)) {
        return create_token_(
            TokenType::reserved_word,
            value
        );
    }

    return create_token_(
        TokenType::identifier,
        value
    );
}

Token StreamingLexer::read_string_literal_() {
    if (tie_possible_) {
        tie_possible_ = false;
        return create_token_(TokenType::tie);
    }

    buffer_ = {};

    while (read_char() != '\"' && !source_is_->eof()) {
        buffer_.sputc(current_char_);
    }

    read_char(); // eat the closing "
    return create_token_(
        TokenType::string_literal,
        buffer_.str()
    );
}

Token StreamingLexer::read_numeric_literal_() {
    if (tie_possible_) {
        tie_possible_ = false;
        return create_token_(TokenType::tie);
    }

    do {
        buffer_.sputc(current_char_);
        read_char();
    } while ((std::iswdigit(current_char_) || current_char_ == '.') && !source_is_->eof());

    return create_token_(
        TokenType::number,
        buffer_.str()
    );
}

Token StreamingLexer::read_linebreak_() {
    unsigned int current_indent = indent_stack_.back();
    
    // eat empty lines
    while (read_char() == '\n' && !source_is_->eof());
    
    // determine next line indent
    // TODO: deal with tab characters
    unsigned int next_line_indent = 0;
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
        return create_token_(TokenType::indent_block_start);
    }

    // if the indent decreased, enter indent closing mode
    while (next_line_indent < indent_stack_.back()) {
        indent_stack_.pop_back();
        indents_to_close_++;
    }
    
    // if the indent went down to a level that was not used before, the code is invalid
    // TODO: be more permissive about this, allow gnu-style braces for example.
    //       Maybe we should consider if the next character is a block-character 
    //       and give a warning otherwise
    // The problem is: which way do we default or should we start a new block?
    // All options seem like possible causes for confusion
    if (next_line_indent != indent_stack_.back()) {
        return create_token_(
            TokenType::indent_error_fatal,
            "Mismatched indent"
        );
    }

    indents_to_close_--;
    return create_token_(
        TokenType::indent_block_end,
        ""
    );
}

Token StreamingLexer::read_pragma() {
    buffer_ = {};

    // eat initial whitespace
    while (read_char() == ' ');

    while (current_char_ != '\n' && !source_is_->eof()) {
        buffer_.sputc(current_char_);
        read_char();
    }

    read_char(); // eat the closing \n
    return create_token_(
        TokenType::pragma,
        buffer_.str()
    );
}

// Reduce redundant semicolons
Token StreamingLexer::collapsed_semicolon_token_() {
    switch (prev_token_type_) {
        case TokenType::indent_block_start:
        case TokenType::indent_block_end:
        case TokenType::semicolon:
        case TokenType::bof:
            return get_token_();
        
        default:
            return create_token_(TokenType::semicolon);
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
