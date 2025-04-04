#include "parser.hh"

#include "config.hh"

Parser::Parser(StreamingLexer* lexer, std::ostream* error_stream):
lexer_(lexer),
errs_(error_stream) 
{}

#include <iostream>
void Parser::run() {
    while (true) {
        get_token();

        switch (current_token_.type) {
            case TokenType::eof:
                return;
                
            case TokenType::unknown:
                // syntax error
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
                if (current_token_.value == "let")
                    parse_let_statement();
                
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

Token Parser::get_token() {
    return current_token_ = lexer_->get_token();
}

void Parser::print_error(const std::string& location, const std::string& message) {
    *errs_ 
        << location << line_col_padding(location.size()) 
        << "error: " << message << '\n';
}

void Parser::print_error(const std::string& message) {
    print_error(current_token_.get_location(), message);
}

bool Parser::identifier_exists(const std::string& identifier) {
    return identifiers_.find(identifier) != identifiers_.end();
}

void Parser::create_identifier(const std::string& identifier, AST::Expression* expression = nullptr) {

    identifiers_.insert({identifier, expression}); //!!!
}

// how to signal failure
AST::Expression* Parser::parse_expression() {

    switch (current_token_.type) {
        case TokenType::identifier:
            // check if exists
                // if does, check if has a value
                // if does, copy over
                    // else leave hanging
            // else leave hanging
        case TokenType::number:
            // create number value
            return {};
            
        case TokenType::char_token:
            switch (current_token_.value.at(0)) {
                case '(':
                    // parse parenthese expression
                case '[':
                    // parse brace expression
                case '{':
                    // parse bracket expression
                default:
                    return {};
            }

        default:
            print_error("unexpected !tokentype! in expression"); // !!!
    }

    expressions_.push_back(std::make_unique<AST::Expression>());
    return expressions_.back().get();
}

// parse_parentheses()
// parse_braces()
// parse_brackets()

// parse_binop_expression()
// parse_function_call()

void Parser::parse_let_statement() {
    switch (get_token().type) {
        case TokenType::whitespace:
            return parse_let_statement();

        case TokenType::identifier:
            // TODO: check lower case here
            {
                std::string name = current_token_.value;
                
                // check if name already exists
                if (identifier_exists(name)) {
                    print_error("attempting to redefine identifier " + name);
                    return;
                }

                // eat whitespace and indent block starts
                do {
                    get_token();
                } while (current_token_.type == TokenType::whitespace || current_token_.type == TokenType::indent_block_start);

                switch (current_token_.type) {
                    case TokenType::semicolon:
                        create_identifier(name);
                        return;
                    case TokenType::operator_t:
                        if (current_token_.value == "=") {
                            break;
                        }

                    default:
                        // TODO: define toketype to string conversion for these messages
                        print_error(
                            "unexpected " + current_token_.get_str() + 
                            " in let-statement, expected an assignment operator");
                        return;
                }

                while(get_token().type == TokenType::whitespace);
                
                // failure mechanism for this
                AST::Expression* expression = parse_expression();

                create_identifier(name, expression);
            }

        default:
            print_error("expected identifier in a let-statement");
            return;
    }
}

