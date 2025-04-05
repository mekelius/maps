#include "parser.hh"

#include "config.hh"

Parser::Parser(StreamingLexer* lexer, std::ostream* error_stream):
lexer_(lexer),
errs_(error_stream) {
    ast_ = std::make_unique<AST::AST>();
}

std::unique_ptr<AST::AST> Parser::run() {
    *errs_ << "\n" << "--- START PARSING ---" << "\n\n";

    // AST::Expression* hello_call = ast_->create_expression(AST::ExpressionType::call);
    // AST::Expression* hello_string = ast_->create_expression(AST::ExpressionType::string_literal);

    // hello_string->string_value = "Hellojaajaa";
    // hello_call->call_expr = {"puts", { hello_string }};

    // ast_->entry_point = hello_call;
    // finished_ = true;

    while (!finished_) {
        get_token();

        switch (current_token_.type) {
            case TokenType::eof:
                finished_ = true;
                break;
                
            case TokenType::reserved_word:
                if (current_token_.value == "let")
                    parse_let_statement();
                break;
                
            // TODO
            default:
            case TokenType::identifier:
            case TokenType::operator_t:
            case TokenType::char_token:
            case TokenType::indent_block_start:
            case TokenType::indent_block_end:
                print_info("parsing of " + current_token_.get_str() + " to be implemented");
                break;
            
            // ---- errors -----
            case TokenType::string_literal:
            case TokenType::number:
                print_error("unexpected " + current_token_.get_str() + " in global context");
                break;
            case TokenType::indent_error_fatal:
                print_error("incorrect indentation");
                break;
            case TokenType::unknown:
                print_error("unknown token type");
                break;

            // ----- types ignored in global context -----
            case TokenType::whitespace:                
            case TokenType::bof:
            case TokenType::semicolon:
                break;
        }
    }

    print_parsing_complete();
    return finalize_parsing();
}

Token Parser::get_token() {
    return current_token_ = lexer_->get_token();
}

void Parser::declare_invalid() {
    program_valid_ = false;
}

std::unique_ptr<AST::AST> Parser::finalize_parsing() {
    if (identifier_exists("main")) {
            ast_->entry_point = identifiers_.find("main")->second;
    }

    ast_->valid = program_valid_;
    return std::move(ast_);
}


// ----- OUTPUT HELPERS -----

void Parser::print_error(const std::string& location, const std::string& message) const {
    *errs_ 
        << location << line_col_padding(location.size()) 
        << "error: " << message << "\n";
}

void Parser::print_error(const std::string& message) const {
    print_error(current_token_.get_location(), message);
}

void Parser::print_info(const std::string& location, const std::string& message) const {
    *errs_
        << location << line_col_padding(location.size()) 
        << "info:  " << message << '\n';
}

void Parser::print_info(const std::string& message) const {
    print_info(current_token_.get_location(), message);
}

void Parser::print_parsing_complete() const {
    *errs_ << "\n" << "--- PARSING COMPLETE ---" << "\n" << std::endl;
    return;
}

// ----- IDENTIFIERS -----

bool Parser::identifier_exists(const std::string& identifier) const {
    return identifiers_.find(identifier) != identifiers_.end();
}

void Parser::create_identifier(const std::string& identifier, AST::Expression* expression) {

    identifiers_.insert({identifier, expression});
}


// ----- PARSING -----

// how to signal failure
AST::Expression* Parser::parse_expression() {

    switch (current_token_.type) {
        case TokenType::identifier:
            if (current_token_.value == "print") {
                return parse_call_expression();
            }
            // check if exists
                // if does, check if has a value
                // if does, copy over
                    // else leave hanging
            // else leave hanging
        case TokenType::number:
            // create number value
            return {};
        
        case TokenType::string_literal: {
                auto expr = ast_->create_expression(AST::ExpressionType::string_literal);
                expr->string_value = current_token_.value;
                return expr;
            }
            
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

    return ast_->create_expression(AST::ExpressionType::string_literal);
}

AST::Expression* Parser::parse_call_expression() {
    std::string callee = current_token_.value;

    switch (get_token().type) {
        case TokenType::whitespace:
            break;
        default:
            print_error("bad call syntax");
            return nullptr; //!!! crash
    }
    get_token();

    AST::Expression* arg = parse_expression();

    auto expr = ast_->create_expression(AST::ExpressionType::call);
    expr->call_expr = {callee, {arg}};
    return expr;

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
                        // !!! now creating string literals like dumb
                        // create_identifier(name, ast_->create_expression(AST::ExpressionType::string_literal));
                        return;
                    case TokenType::operator_t:
                        if (current_token_.value == "=") {
                            break;
                        }

                    default:
                        print_error(
                            "unexpected " + current_token_.get_str() + 
                            " in let-statement, expected an assignment operator");
                        return;
                }

                while(get_token().type == TokenType::whitespace);
                
                // failure mechanism for this
                AST::Expression* expression = parse_expression();

                create_identifier(name, expression);
                return;
            }

        default:
            print_error("expected identifier in a let-statement");
            return;
    }
}

