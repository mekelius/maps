#include "parser.hh"

#include "config.hh"

Parser::Parser(StreamingLexer* lexer, std::ostream* error_stream):
lexer_(lexer),
errs_(error_stream) {
    ast_ = std::make_unique<AST::AST>();
    get_token();
}

std::unique_ptr<AST::AST> Parser::run() {
    
    AST::init_builtin_callables(*ast_);

    while (!finished_) {
        get_token();

        switch (current_token().type) {
            case TokenType::eof:
                finished_ = true;
                break;
                
            case TokenType::reserved_word:
                if (current_token().value == "let")
                    parse_let_statement();
                break;
                
            // TODO
            default:
            case TokenType::identifier:
            case TokenType::operator_t:
            case TokenType::char_token:
            case TokenType::indent_block_start:
            case TokenType::indent_block_end:
                print_info("parsing of " + current_token().get_str() + " to be implemented");
                break;
            
            // ---- errors -----
            case TokenType::string_literal:
            case TokenType::number:
                print_error("unexpected " + current_token().get_str() + " in global context");
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
    token_buf_[which_buf_slot_ % 2] = lexer_->get_token();
    which_buf_slot_++;
    return current_token();
}

Token Parser::current_token() const {
    return token_buf_[which_buf_slot_ % 2];
}

Token Parser::peek() const {
    return token_buf_[(which_buf_slot_+1) % 2];
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
    print_error(current_token().get_location(), message);
}

void Parser::print_info(const std::string& location, const std::string& message) const {
    *errs_
        << location << line_col_padding(location.size()) 
        << "info:  " << message << '\n';
}

void Parser::print_info(const std::string& message) const {
    print_info(current_token().get_location(), message);
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

    switch (current_token().type) {
        case TokenType::identifier: {
            Token next_token = peek();
            switch (next_token.type) {
                case TokenType::char_token:
                    if (
                        next_token.value == "("  ||
                        next_token.value == "["  ||
                        next_token.value == "{"  ||
                        next_token.value == "::" ||
                        next_token.value == "."
                    )
                        return parse_access_expression();

                case TokenType::identifier:
                case TokenType::operator_t:
                case TokenType::whitespace:
                case TokenType::number:
                case TokenType::string_literal:
                    return parse_termed_expression();    

                case TokenType::eof:
                case TokenType::semicolon:
                case TokenType::indent_block_end:
                    return parse_identifier_expression();

                default:
                    return parse_identifier_expression();
            }
        }

        case TokenType::number: 
            return parse_numeric_literal();

        case TokenType::string_literal: 
            return parse_string_literal();
            
        case TokenType::char_token:
            switch (current_token().value.at(0)) {
                case '(': 
                    return parse_parenthesized_expression();

                case '[': {
                    AST::Expression* expression = parse_expression();
                    if (current_token().type != TokenType::char_token || current_token().value.at(0) != ']') {
                        print_error("Mismatched brackets");
                        get_token();
                    }
                    return expression;
                }

                case '{':{
                    AST::Expression* expression = parse_expression();
                    if (current_token().type != TokenType::char_token || current_token().value.at(0) != '}') {
                        print_error("Mismatched curly braces");
                        get_token();
                    }
                    return expression;
                }
                    // parse bracket expression
                default:
                    return ast_->create_expression(AST::ExpressionType::not_implemented);
            }

        default:
            print_error("unexpected " + current_token().get_str() + " in expression");
    }

    return ast_->create_expression(AST::ExpressionType::string_literal);
}

// expects to be called with the first term as current
AST::Expression* Parser::parse_termed_expression() {
    AST::Expression* expression = ast_->create_expression(AST::ExpressionType::termed_expression);

    expression->terms.push_back(parse_term());

    while (true) {
        get_token();
        switch (current_token().type) {
            case TokenType::eof:
            case TokenType::indent_block_end:
            case TokenType::semicolon:
                return expression;

            case TokenType::char_token:
                switch (current_token().value.at(0)) {
                    case ')':
                    case ']':
                    case '}':
                        return expression;

                    case '(':
                    case '[':
                    case '{':
                        expression->terms.push_back(parse_term());
                }

            case TokenType::string_literal:
            case TokenType::number:
            case TokenType::identifier:
            case TokenType::operator_t:
            case TokenType::indent_block_start:
                expression->terms.push_back(parse_term());

            case TokenType::whitespace:
                // TODO: do ties
                continue;
            default:
                expression->terms.push_back(ast_->create_expression(AST::ExpressionType::not_implemented));
        }
    }
}

AST::Expression* Parser::parse_identifier_expression() {
    AST::Expression* expression = ast_->create_expression(AST::ExpressionType::unresolved_identifier);
    expression->string_value = current_token().value;
    return expression;
}

AST::Expression* Parser::parse_access_expression() {
    // eat until the closing character
    return ast_->create_expression(AST::ExpressionType::not_implemented);
}

// expects to be called with the opening parenthese as the current_token_
AST::Expression* Parser::parse_parenthesized_expression() {
    get_token();
    AST::Expression* expression = parse_expression();
    if (current_token().type != TokenType::char_token || current_token().value.at(0) != ')') {
        print_error("Mismatched parentheses");
        get_token();
    }
    return expression;
}

AST::Expression* Parser::parse_mapping_literal() {
    // eat until the closing character
    return ast_->create_expression(AST::ExpressionType::not_implemented);
}

AST::Expression* Parser::parse_string_literal() {
    auto expr = ast_->create_expression(AST::ExpressionType::string_literal);
    expr->string_value = current_token().value;
    return expr;
}

AST::Expression* Parser::parse_numeric_literal() {
    auto expression = ast_->create_expression(AST::ExpressionType::numeric_literal, &AST::NumberLiteral);
    expression->string_value = current_token().value;
    return expression;
}

// Expects not to be called if the current token is not parseable into a term
AST::Expression* Parser::parse_term() {
    switch (current_token().type) {
        case TokenType::identifier:
            // TodO: revamp access expressions
            return parse_identifier_expression();
        case TokenType::string_literal:
            return parse_string_literal();
        case TokenType::number:
            return parse_numeric_literal();

        case TokenType::char_token:
            switch (current_token().value.at(0)) {
                case '(': 
                    return parse_parenthesized_expression();
                case '[':
                case '{':
                    return parse_mapping_literal();
                default:
                    assert(false && "Parser::parse_term called with a non-term token");
            }
            
        default:
            assert(false && "Parser::parse_term called with a non-term token");
    }
}

// this means that the previous token was an identifier or an operator
AST::Expression* Parser::parse_call_expression(const std::string& callee) {
    // TODO: handle tuple arg lists
    assert(
        (current_token().type == TokenType::identifier)
        && "Parser::parse_call_expression(Token) called when the current_token() was not an identifier"
    );

    // check that the function exists here, or mark as hanging
    auto call = ast_->create_expression(AST::ExpressionType::call);

    // parse the following tokens as an argument list
    call->call_expr = {callee, parse_argument_list()};
    return call;
}

AST::Expression* Parser::parse_call_expression(AST::Expression* callee, const std::vector<AST::Type*>& signature) {
    assert(callee && "Passed nullptr to Parser::parse_call_expression");

    if (signature.size() > 0)
        print_error("Parser::parse_call_expression doesn't know how to handle type signatures yet, ignoring the signature");
    
    switch (callee->expression_type) {
        case AST::ExpressionType::unresolved_identifier:
            return parse_call_expression(callee->string_value);

        case AST::ExpressionType::native_function:
        case AST::ExpressionType::native_operator: { // why the heck not
        
            auto maybe_callee = lookup_identifier(current_token().value);
            // TODO: ideally assert here that the callee exists as a builtin
            auto [callee_identifier, _2, arg_types] = **maybe_callee;
            unsigned int arity = arg_types.size();
            
            if (arity == 0) {
                auto expr = ast_->create_expression(AST::ExpressionType::call);
                expr->call_expr = {current_token().value, {}};
                return expr;
            }
            
            if (arity != 1) {
                print_error("CAN'T YET PARSE ARG LISTS LONGER THAN 1");
                declare_invalid();
                return ast_->create_expression(AST::ExpressionType::call);
            }
            
            // !!! this arg stuff is something to be deferred, since we might later get polymorphic defs with different arity
            // !!! On the other hand, with built ins and such we already know the arity
            // Therefore, builtins are different semantically?
            // We must make sure that whether something is a builtin or not does not affect semantics
            // Yes, with inference we must concretize types as soon as we can to reduce the complexity

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
        default:
            assert(false && "Parser::parse_call_expression called with an expression that's not callable");
    }
}


AST::Expression* Parser::parse_operator_expression(Token previous_token) {
    return ast_->create_expression(AST::ExpressionType::not_implemented);
}

// TODO: make arg lists an expressiontype
std::vector<AST::Expression*> Parser::parse_argument_list() {
    assert(false && "Not ready");
    return {};    
}

void Parser::parse_let_statement() {
    switch (get_token().type) {
        case TokenType::whitespace:
            return parse_let_statement();

        case TokenType::identifier:
            // TODO: check lower case here
            {
                std::string name = current_token().value;
                
                // check if name already exists
                if (identifier_exists(name)) {
                    print_error("attempting to redefine identifier " + name);
                    return;
                }

                // eat whitespace and indent block starts
                do {
                    get_token();
                } while (current_token().type == TokenType::whitespace);

                switch (current_token().type) {
                    case TokenType::semicolon:
                        // !!! now creating string literals like dumb
                        // create_identifier(name, ast_->create_expression(AST::ExpressionType::string_literal));
                        return;
                    case TokenType::operator_t:
                        if (current_token().value == "=") {
                            break;
                        }

                    default:
                        print_error(
                            "unexpected " + current_token().get_str() + 
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
            print_error("unexpected token: " + current_token().get_str() + " in let statement");
            return;
    }
}

