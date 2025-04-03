#include "parser.hh"

DirectParser::DirectParser(StreamingLexer* lexer, IRGenHelper* ir_gen):
lexer_(lexer),
ir_gen_(ir_gen) {
}

#include <iostream>
void DirectParser::run() {
    while (true) {
        current_token_ = lexer_->get_token();

        switch (current_token_.type) {
            case TokenType::eof:
                return;
                
            case TokenType::unknown:
                break;

            case TokenType::identifier:
                break;

            case TokenType::number:
                break;

            case TokenType::operator_t:
                break;

            case TokenType::string_literal:
                break;

            case TokenType::whitespace:
                break;

            case TokenType::char_token:
                break;

            case TokenType::reserved_word:
                break;
            
            case TokenType::indent_block_start:
                break;
            
            case TokenType::indent_block_end:
                break;
            
            case TokenType::semicolon:
                break;

            case TokenType::indent_error_fatal:
                break;

            case TokenType::bof:
                break;

            default:
                break;
        }
    }
}