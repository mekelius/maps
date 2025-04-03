#include "parser.hh"

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
            
            case TokenType::indent_block_start:
                std::cerr << "token: indent block start" << std::endl;
                break;
            
            case TokenType::indent_block_end:
                std::cerr << "token: indent block end " << current_token_.int_value << std::endl;
                break;
            
            case TokenType::semicolon:
                std::cerr << "token: ;" << std::endl;
                break;

            case TokenType::indent_error_fatal:
                std::cerr << "syntax error at line " << current_token_.int_value << ": " << current_token_.value << std::endl;
                break;


            default:
                std::cerr << "unhandled token type" << std::endl;
                break;
        }

        current_token_ = lexer_->get_token();
    }
}