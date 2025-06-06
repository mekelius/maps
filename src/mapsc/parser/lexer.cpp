#include "lexer.hh"

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cwctype>
#include <sstream>
#include <string>

#include "mapsc/logging.hh"
#include "mapsc/words.hh"


namespace Maps {

using Log = LogInContext<LogContext::lexer>;

// ----- Public methods -----
Lexer::Lexer(std::istream* source_is, SourceFileID source_id)
:source_is_(source_is), source_id_(source_id) {
    read_char();
}

Token Lexer::get_token() {
    Token token = get_token_();

    Log::debug_extra(prev_token_.location) << "TOKEN: " << prev_token_ << Endl;
    
    // a bit of a hack to keep the outputs in sync
    prev_token_ = token;
    tie_possible_ = is_right_tieable_token(token);

    return token;
}

// ----- Private methods -----

// Read a character from the input stream
char Lexer::read_char() {
    if (source_is_->eof())
        return EOF;

    if (current_char_ == '\n') {
        current_col_ = 1;
        current_line_++;
    } else {
        current_col_++;
    }

    source_is_->get(current_char_);

    // just pretend like CRLF doesn't exist
    // !!! untested on a windows machine
    return (current_char_ != '\r') ? current_char_ : read_char();
}

char Lexer::peek_char() {
    return source_is_->peek();
}

SourceLocation Lexer::current_location() const {
    return SourceLocation{static_cast<int>(current_token_start_line_),
        static_cast<int>(current_token_start_col_), source_id_};
}


Token Lexer::create_token(TokenType type) {
    return Token{type, current_location()};
}

Token Lexer::create_token(TokenType type, const std::string& value) {
    return Token{type, value, current_location()};
}

// ----- PRODUCTION RULES -----

Token Lexer::get_token_() {
    if (indents_to_close_ > 0) {
        indents_to_close_--;
        return create_token(TokenType::indent_block_end);
    }

    current_token_start_col_ = current_col_;
    current_token_start_line_ = current_line_;

    buffer_ = {};

    if (source_is_->eof())
        return create_token(TokenType::eof);

    switch (current_char_) {
        case EOF:
            return create_token(TokenType::eof);

        // handle string literals
        // TODO: handle '
        // TODO: string interpolation
        case '\"':
            return read_string_literal();

        // handle whitespace
        case ' ':
            while (read_char() == ' ');
            tie_possible_ = false;
            return get_token_();

        case '#':
            return read_pragma();
    
        case '/':
            if (peek_char() == '/' || peek_char() == '*') {
                read_char();
                read_and_ignore_comment();
                return get_token_();
            }
            
            return read_operator();

        case '-':
        case '=':
            return read_operator();

        case '(':
            if (tie_possible_) {
                tie_possible_ = false;
                return create_token(TokenType::tie);
            }
            read_char();
            return create_token(TokenType::parenthesis_open);

        case '[':
            read_char();
            if (tie_possible_) {
                tie_possible_ = false;
                return create_token(TokenType::tie);
            }
            read_char();
            return create_token(TokenType::bracket_close);

        case '{':
            if (tie_possible_) {
                tie_possible_ = false;
                return create_token(TokenType::tie);
            }
            read_char();
            return create_token(TokenType::curly_brace_open);

        case ')':
            read_char();
            return create_token(TokenType::parenthesis_close);
        case ']':
            read_char();
            return create_token(TokenType::bracket_close);
        case '}':
            read_char();
            return create_token(TokenType::curly_brace_close);
        case ',':
            read_char();
            return create_token(TokenType::comma);
        case '\\':
            read_char();
            return create_token(TokenType::lambda);
        
        case '\n':
            return read_linebreak();

        case '?':
            if (is_operator_glyph(peek_char()))
                return read_operator();

            return create_token(TokenType::question_mark);

        case ';':
            while(read_char() == ';' && !source_is_->eof());
            return collapsed_semicolon_token();

        case ':':
            read_char();

            if (current_char_ == ':') {
                read_char();
                return create_token(TokenType::double_colon);
            }

            return create_token(TokenType::colon);

        case '_':
            return read_identifier();

        default:
            // handle identifiers
            // TODO: handle suffixes
            if (std::isalpha(current_char_)) {
                // actually we can tie type identifiers
                // type identifiers can't be tied
                // if (!islower(current_char_))
                //     tie_possible_ = false;

                return read_identifier();
            }

            // handle numerics
            // TODO: how to support spaces in numerics?
            // TODO: hex and oct
            if (std::iswdigit(current_char_))
                return read_numeric_literal();

            // handle operators
            if (is_operator_glyph(current_char_))
                return read_operator();

            // unknown token
            assert(false && "unhandled token type in Lexer::get_token_()");
            read_char();
            return create_token(TokenType::syntax_error);
    }
}

Token Lexer::read_identifier() {
    if (tie_possible_) {
        tie_possible_ = false;
        return create_token(TokenType::tie);
    }

    buffer_ = {};
    buffer_.sputc(current_char_);
    read_char();
    
    while (is_allowed_in_identifiers(current_char_) && !source_is_->eof()) {
        buffer_.sputc(current_char_);
        read_char();
    }

    std::string value = buffer_.str();

    if (auto token_type = lookup_reserved_word_token_type(value))
        return create_token(*token_type);

    if (std::isupper(value.at(0)))
        return create_token(TokenType::type_identifier, value);

    return create_token(TokenType::identifier, value);
}

Token Lexer::read_operator() {
    if (tie_possible_) {
        tie_possible_ = false;
        return create_token(TokenType::tie);
    }

    // cannot reset the buffer, since there might be an initial '/' there
    while (is_operator_glyph(current_char_)) {
        buffer_.sputc(current_char_);
        read_char();
        if (source_is_->eof())
            return create_token(TokenType::eof);
    }

    auto result = buffer_.str();
    if (result == "->" || result == "=>")
        return create_token(TokenType::arrow_operator, result);

    return create_token(TokenType::operator_t, result);
}

Token Lexer::read_string_literal() {
    if (tie_possible_) {
        tie_possible_ = false;
        return create_token(TokenType::tie);
    }

    buffer_ = {};

    while (read_char() != '\"' && !source_is_->eof()) {
        buffer_.sputc(current_char_);
    }

    if (current_char_ != '\"') {
        if (source_is_->eof()) {
            Log::error(current_location()) << "Unexpected eof during string literal" << Endl;
        } else {
            Log::compiler_error(current_location()) <<
                "Something unexpected happened during string literal lexing";
        }
        return create_token(TokenType::syntax_error, "string literal missing closing quote");
    }

    read_char(); // eat the closing "
    return create_token(TokenType::string_literal, buffer_.str());
}

Token Lexer::read_numeric_literal() {
    if (tie_possible_) {
        tie_possible_ = false;
        return create_token(TokenType::tie);
    }

    do {
        buffer_.sputc(current_char_);
        read_char();
    } while ((std::iswdigit(current_char_) || current_char_ == '.') && !source_is_->eof());

    return create_token(TokenType::number, buffer_.str());
}

Token Lexer::read_linebreak() {
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
        return read_linebreak();

    // deal with possible comments
    if (current_char_ == '/') {
        char peeked_char = source_is_->peek();
        // it's a comment
        if (peeked_char == '/' || peeked_char == '*') {
            read_char(); // read_and_ignore comment expects us to reat the initial '/'
            read_and_ignore_comment();
            // !!!: multi-line comments will mess with indentation here
            return read_linebreak();
        }
    }

    if (source_is_->eof())
        return create_token(TokenType::eof);

    // if the indent didn't change, just insert a semicolon
    if (next_line_indent == current_indent) 
        return collapsed_semicolon_token();

    // if the indent increased, start a new level
    if (next_line_indent > current_indent) {
        indent_stack_.push_back(next_line_indent);
        return create_token(TokenType::indent_block_start);
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
    if (next_line_indent != indent_stack_.back())
        return create_token(TokenType::indent_error_fatal);

    indents_to_close_--;
    return create_token(TokenType::indent_block_end);
}

Token Lexer::read_pragma() {
    buffer_ = {};

    // eat initial whitespace
    while (read_char() == ' ');

    while (current_char_ != '\n' && !source_is_->eof()) {
        buffer_.sputc(current_char_);
        read_char();
    }

    read_char(); // eat the closing \n
    return create_token(TokenType::pragma, buffer_.str());
}

// Reduce redundant semicolons
Token Lexer::collapsed_semicolon_token() {
    switch (prev_token_.token_type) {
        case TokenType::indent_block_start:
        case TokenType::indent_block_end:
        case TokenType::semicolon:
        case TokenType::dummy:
            return get_token_();
        
        default:
            return create_token(TokenType::semicolon);
    }
}

// when calling this, the caller must eat the first '/' of the comment
// the caller is also responsible for checking that current_char_ is either '/' or '*'
void Lexer::read_and_ignore_comment() {
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
    // !!! NOTE: multi-line comments may mess with indentation, but can't be bothered to 
    // think about that atm
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

} // namespace Maps