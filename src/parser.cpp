#include "parser.hh"

#include "config.hh"

Parser::Parser(StreamingLexer* lexer, std::ostream* error_stream):
lexer_(lexer),
errs_(error_stream) {
    ast_ = std::make_unique<AST::AST>();
}

std::unique_ptr<AST::AST> Parser::run() {
    
    AST::init_builtin_callables(*ast_);

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

    return finalize_parsing();
}

Token Parser::get_token() {
    return current_token_ = lexer_->get_token();
}

void Parser::declare_invalid() {
    program_valid_ = false;
}

std::unique_ptr<AST::AST> Parser::finalize_parsing() {
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

// ----- IDENTIFIERS -----

bool Parser::identifier_exists(const std::string& identifier) const {
    return !(ast_->name_free(identifier));
}

void Parser::create_identifier(const std::string& identifier, AST::Expression* expression) {
    assert(expression && "Parser tried to create an identifier with nullptr as the expression");

    ast_->create_callable(identifier, expression);
}

std::optional<AST::Callable*> Parser::lookup_identifier(const std::string& identifier) {
    return ast_->get_identifier(identifier);
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
            return ast_->create_expression(AST::ExpressionType::not_implemented);
        
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
                    return ast_->create_expression(AST::ExpressionType::not_implemented);
            }

        default:
            print_error("unexpected " + current_token_.get_str() + " in expression");
    }

    return ast_->create_expression(AST::ExpressionType::string_literal);
}

AST::Expression* Parser::parse_call_expression() {
    // check that the function exists here, or mark as hanging
    auto maybe_callee = lookup_identifier(current_token_.value);

    if (!maybe_callee) {
        // mark somehow as hanging and return later
        print_error("HOISTING NEEDED, tried to call: \"" + current_token_.value + "\" before definition");
        declare_invalid();
        return ast_->create_expression(AST::ExpressionType::call);
    }

    auto [callee_identifier, _2, arg_types] = **maybe_callee;
    unsigned int arity = arg_types.size();

    if (arity == 0) {
        auto expr = ast_->create_expression(AST::ExpressionType::call);
        expr->call_expr = {current_token_.value, {}};
        return expr;
    }

    if (arity != 1) {
        print_error("CAN'T YET PARSE ARG LISTS LONGER THAN 1");
        declare_invalid();
        return ast_->create_expression(AST::ExpressionType::call);
    }

    switch (get_token().type) {
        case TokenType::whitespace:
            break;
        default:
            print_error("bad call syntax");
            declare_invalid();
            return ast_->create_expression(AST::ExpressionType::call); //!!! crash
    }
    get_token(); //eat whitespace

    AST::Expression* arg = parse_expression();

    auto expr = ast_->create_expression(AST::ExpressionType::call);
    expr->call_expr = {callee_identifier, {arg}};
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

