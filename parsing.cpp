#include "parsing.hh"

#include <cctype>
#include <sstream>
#include <string>
#include <algorithm>

bool is_operator_glyph(char glyph) {
    return OPERATOR_GLYPHS.find(glyph) != std::string::npos;
}

bool is_reserved_word(const std::string& word) {
    return std::find(RESERVED_WORDS.begin(), RESERVED_WORDS.end(), word) != RESERVED_WORDS.end();
}

const Token EOF_TOKEN = {
    TokenType::eof
};

const Token UNKNOWN_TOKEN = {
    TokenType::unknown
};

// ----- StreamingLexer -----
StreamingLexer::StreamingLexer(std::istream* source_is): 
source_is_(source_is) {
    read_char();
}

char StreamingLexer::read_char() {
    source_is_->get(current_char_);
    return current_char_;
}

Token StreamingLexer::get_token() {
    std::stringbuf buffer = {};

    if (source_is_->eof())
        return EOF_TOKEN;

    switch (current_char_) {
        case EOF:
            return EOF_TOKEN;

        // TODO: handle :, (), [], {}, +-*/=, += =+ -= =- ++ --, ., ->, ~>, =>

        // handle string literals
        // TODO: handle '
        // TODO: string interpolation
        case '\"':
            while (read_char() != '\"') {
                buffer.sputc(current_char_);
            }

            read_char();
            return {
                TokenType::string_literal,
                buffer.str()
            };

        // handle whitespace
        case ' ':
            while (current_char_ == ' ') {
                read_char();
            }

            return {
                TokenType::whitespace
            };
    
        case '/':
            read_char();

            // handle comments
            // single-line comment
            if (current_char_ == '/') {
                while (read_char() != '\n') {
                    if (source_is_->eof())
                        return EOF_TOKEN;
                }
                return get_token();
            }
            
            // multi-line comment
            // NOTE: multi-line comments may mess with indentation, but can't be bothered to think about that atm
            if (current_char_ == '*') {
                bool found_closing = false;

                read_char();
                
                while (!found_closing) {
                    read_char();
                    if (source_is_->eof())
                        return EOF_TOKEN;
                    if (current_char_ == '*') {
                        if (read_char() == '/')
                            found_closing = true;
                    }
                }

                return get_token();
            }

            // handle operator
            buffer.sputc('/');
            while (is_operator_glyph(current_char_)) {
                buffer.sputc(current_char_);
                read_char();
            }
            return {
                TokenType::operator_t,
                buffer.str()
            };

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
        
        case ';':
        case '\n':
            // TODO: handle indentation
            read_char();
            while(current_char_ == ';' || current_char_ == '\n') {
                read_char();
            }

            return { TokenType::char_token, ";" };

        default:
            // handle identifiers
            // TODO: handle type identifiers
            {
                while ((std::isalpha(current_char_) && std::islower(current_char_)) || current_char_ == '_' || current_char_ == '?' || current_char_ == '!') {
                    buffer.sputc(current_char_);
                    read_char();
                }
                std::string value = buffer.str();
                if (value.size() > 0) {
                    if (is_reserved_word(value))
                        return {
                            TokenType::reserved_word,
                            value
                        };

                    return {
                        TokenType::identifier,
                        value
                    };
                }
            }

            // handle numerics
            // TODO: how to support spaces in numerics?
            // TODO: hex and oct
            {
                while (std::iswdigit(current_char_) || current_char_ == '.') {
                    buffer.sputc(current_char_);
                    read_char();
                }
                std::string value = buffer.str();
                if (value.size() > 0) {
                    return {
                        TokenType::number,
                        value
                    };
                }
            }

            // handle operators
            {
                while (is_operator_glyph(current_char_)) {
                    buffer.sputc(current_char_);
                    read_char();
                }
                std::string value = buffer.str();
                if (value.size() > 0) {
                    return {
                        TokenType::operator_t,
                        value
                    };
                }
            }

            // unknown token
            read_char();
            return UNKNOWN_TOKEN;
    }
}

DirectParser::DirectParser(StreamingLexer* lexer, IRGenHelper* ir_gen):
lexer_(lexer),
ir_gen_(ir_gen) {
}

#include <iostream>
void DirectParser::run() {
    current_token_ = lexer_->get_token();
    
    while (true) {
        switch (current_token_.type) {
            case TokenType::eof:
                return;
            case TokenType::unknown:
                std::cerr << "token: unknown" << std::endl;
                break;
            case TokenType::identifier:
                std::cerr << "token: identifier " << current_token_.value << std::endl;
                break;
            case TokenType::number:
                std::cerr << "token: numeric literal " << current_token_.value << std::endl;
                break;
            case TokenType::operator_t:
                std::cerr << "token: operator " << current_token_.value << std::endl;
                break;
            case TokenType::string_literal:
                std::cerr << "token: string literal \"" << current_token_.value << "\"" << std::endl;
                break;
            case TokenType::whitespace:
                std::cerr << "token: whitespace " << std::endl;
                break;
            case TokenType::char_token:
                std::cerr << "token: " << current_token_.value << std::endl;
                break;
            case TokenType::reserved_word:
                std::cerr << "token: reserved word " << current_token_.value << std::endl;
                break;
            default:
                std::cerr << "unhandled token type" << std::endl;
                break;
        }

        current_token_ = lexer_->get_token();
    }
}