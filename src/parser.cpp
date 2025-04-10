/**
 * Recursive descent parser for maps programming language
 * 
 * The convention here (unlike in the lexer) is that every production rule must move the buffer
 * beyond the tokens it consumed.
 * 
 * Same if a token is rejected. 
 */

#include <variant>

#include "parser.hh"
#include "config.hh"

using Logging::LogLevel;
using Logging::MessageType;

// ----- PUBLIC METHODS -----

Parser::Parser(StreamingLexer* lexer, std::ostream* error_stream):
lexer_(lexer),
errs_(error_stream) {
    ast_ = std::make_unique<AST::AST>();
    get_token();
    get_token();
}

std::unique_ptr<AST::AST> Parser::run() {    
    ast_->init_builtin_callables();

    while (current_token().type != TokenType::eof) {
        int prev_buf_slot = which_buf_slot_; // a bit of a hack, an easy way to do the assertion below

        parse_top_level_statement();

        assert(which_buf_slot_ != prev_buf_slot && 
            "Parser::parse_top_level_statement didn't advance the tokenstream");
    }

    return finalize_parsing();
}
// ----- PRIVATE METHODS -----

Token Parser::get_token() {
    token_buf_[which_buf_slot_ % 2] = lexer_->get_token();
    which_buf_slot_++;

    // manage the parentheses and indents etc.
    update_brace_levels(current_token());

    return current_token();
}

Token Parser::current_token() const {
    return token_buf_[which_buf_slot_ % 2];
}

Token Parser::peek() const {
    return token_buf_[(which_buf_slot_+1) % 2];
}

void Parser::update_brace_levels(Token token) {
    switch (token.type) {
        case TokenType::indent_block_start:
            indent_level_++; break;
        case TokenType::indent_block_end:
            indent_level_--; break;

        case TokenType::bracket_open:
            angle_bracket_level_++; break;
        case TokenType::bracket_close:
            angle_bracket_level_--; break;

        case TokenType::curly_brace_open:
            curly_brace_level_++; break;
        case TokenType::curly_brace_close:
            curly_brace_level_--; break;

        case TokenType::parenthesis_open:
            parenthese_level_++; break;
        case TokenType::parenthesis_close:
            parenthese_level_--; break;

        default: return;
    }
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
    if (!LogLevel::has_message_type(MessageType::error))
        return;

    *errs_ 
        << location << line_col_padding(location.size()) 
        << "error: " << message << "\n";
}

void Parser::print_error(const std::string& message) const {
    print_error(current_token().get_location(), message);
}

void Parser::print_info(const std::string& location, const std::string& message, MessageType message_type) const {
    if (!LogLevel::has_message_type(message_type))
        return;

    *errs_
        << location << line_col_padding(location.size()) 
        << "info:  " << message << '\n';
}

void Parser::print_info(const std::string& message,  MessageType message_type) const {
    print_info(current_token().get_location(), message, message_type);
}

// ----- IDENTIFIERS -----

bool Parser::identifier_exists(const std::string& identifier) const {
    return !(ast_->name_free(identifier));
}

void Parser::create_identifier(const std::string& identifier, AST::CallableBody body) {
    ast_->create_callable(identifier, body);
}

std::optional<AST::Callable*> Parser::lookup_identifier(const std::string& identifier) {
    return ast_->get_identifier(identifier);
}

// ########## PARSING ##########

void Parser::handle_pragma() {
    if (current_token().value == "enable mutable globals") {
        ast_->pragmas.mutable_globals = true;

    } else if (current_token().value == "enable top level context") {
        ast_->pragmas.top_level_context = true;
    
    } else if (current_token().value == "script") {
        ast_->pragmas.top_level_context = true;
        ast_->pragmas.mutable_globals = true;
    
    } else {
        print_error("unknown pragma: " + current_token().value);
    }

    get_token();
    print_info("set pragma", MessageType::parser_debug);
}

// --------- STATEMENTS --------

void Parser::parse_top_level_statement() {
    AST::Statement* statement;

    switch (current_token().type) {
        case TokenType::pragma:
            handle_pragma();
            return parse_top_level_statement();

        case TokenType::eof:
            finished_ = true;
            return;

        default:
            // TODO: check pragmas for top-level statement types
            statement = parse_statement();
            if (!std::holds_alternative<AST::EmptyStatement>(*statement))            
                ast_->append_top_level_statement(statement);
            return;
    }
}

AST::Statement* Parser::parse_non_global_statement() {
    switch (current_token().type) {
        case TokenType::pragma:
            print_error("unexpected non top-level pragma");
            reset_to_top_level();
            return ast_->create_statement(AST::BrokenStatement{}); 

        case TokenType::eof:
            finished_ = true;
            return ast_->create_statement(AST::BrokenStatement{});

        default:
            return parse_statement();
    }
}

AST::Statement* Parser::parse_statement() {
    switch (current_token().type) {
        case TokenType::eof:
            finished_ = true;
            return ast_->create_statement(AST::EmptyStatement{});

        case TokenType::reserved_word:
            print_info("scoping not yet implemented");
            if (current_token().value == "let")
                return parse_let_statement();
            
            assert(false && "Unhandled reserved word in Parser::parse_top_level_statement");
            
        case TokenType::identifier:
            if (peek().type == TokenType::operator_t && peek().value == "=") {
                return parse_assignment_statement();
            }
            return parse_expression_statement();

        case TokenType::indent_block_start:
        case TokenType::curly_brace_open:
            return parse_block_statement();

        case TokenType::parenthesis_open:
        case TokenType::bracket_open:
        case TokenType::operator_t:
        case TokenType::string_literal:
        case TokenType::number:
            return parse_expression_statement();

        case TokenType::semicolon:
            get_token(); // eat the semicolon
            return ast_->create_statement(AST::EmptyStatement{});

        // ----- ignored -----
        case TokenType::bof:
            get_token(); // eat the bof
            return ast_->create_statement(AST::EmptyStatement{});
            
        // ---- errors -----
        default:
            print_error("use \"@ enable top level context\" to allow top level evaluation");
            reset_to_top_level();
            return ast_->create_statement(AST::BrokenStatement{});
        
        case TokenType::indent_block_end:
            assert(false && "parse_statement called at indent_block_end");
            return ast_->create_statement(AST::BrokenStatement{});
        
        case TokenType::indent_error_fatal:
            print_error("indent error");
            reset_to_top_level();
            return ast_->create_statement(AST::BrokenStatement{});

        case TokenType::unknown:
            print_error("unknown token");
            reset_to_top_level();
            return ast_->create_statement(AST::BrokenStatement{});
    }
}

AST::Statement* Parser::parse_expression_statement() {
    AST::Expression* expression = parse_expression();
    AST::Statement* statement = ast_->create_statement(AST::ExpressionStatement{expression});

    std::string start_location = current_token().get_location();

    assert(is_block_starter(current_token()) || is_statement_separator(current_token()) && "statement didn't end in a statement separator");
    get_token(); // eat statement separator
    print_info("finished parsing expression statement from " + start_location, MessageType::parser_debug);
    return statement;
}

AST::Statement* Parser::parse_let_statement() {
    switch (get_token().type) {
        case TokenType::identifier:
            // TODO: check lower case here
            {
                std::string name = current_token().value;
                
                // check if name already exists
                if (identifier_exists(name)) {
                    print_error("Attempting to redefine identifier " + name);
                    return ast_->create_statement(AST::IllegalStatement{"Attempting to redefine identifier: " + name});
                }

                get_token();

                if (is_statement_separator(current_token())) {
                    print_info("parsed let statement declaring \"" + name + "\" with no definition", MessageType::parser_debug);
                    
                    get_token(); // eat the semicolon
                    return ast_->create_statement(AST::LetStatement{name});
                }

                if (is_assignment_operator(current_token())) {
                    get_token(); // eat the assignment operator

                    AST::CallableBody body;
                    if (is_block_starter(current_token())) {
                        body = parse_block_statement();
                    } else {
                        body = parse_expression();
                    }

                    AST::Statement* statement = ast_->create_statement(AST::LetStatement{name, body});

                    print_info("parsed let statement", MessageType::parser_debug);
                    return statement;
                }

                get_token();
                print_error("unexpected " + current_token().get_str() + ", in let-statement");
                reset_to_top_level();
                return ast_->create_statement(AST::BrokenStatement{});
            }

        case TokenType::operator_t:
            print_info("operator overloading not yet implemented, ignoring");
            reset_to_top_level();
            return ast_->create_statement(AST::EmptyStatement{});

        default:
            print_error("unexpected token: " + current_token().get_str() + " in let statement");
            reset_to_top_level();
            return ast_->create_statement(AST::BrokenStatement{});
    }
}

AST::Statement* Parser::parse_assignment_statement() {
    assert(current_token().type == TokenType::identifier 
        && "parse_assignment_statement called with current_token that was not an identifier");

    std::string start_location = current_token().get_location();
    std::string name = current_token().value;

    get_token();
    // TODO: change '=' to it's own TokenType
    // !!! in any case assignment operators should live in their own category
    assert(is_assignment_operator(current_token())
        && "assignment statement second token not an assignment operator");
   
    get_token(); // eat '='

    AST::Statement* inner_statement = parse_statement();
    AST::Statement* statement = ast_->create_statement(AST::AssignmentStatement{name, inner_statement});

    // expect parse_statement to eat the semicolon etc.

    print_info("finished parsing assignment statement from " + start_location, MessageType::parser_debug);
    return statement;
}

AST::Statement* Parser::parse_block_statement() {
    assert(is_block_starter(current_token())
        && "parse_block_statement called with current token that is not a block starter");

    unsigned int indent_at_start = indent_level_;
    unsigned int curly_brace_at_start = curly_brace_level_;
    std::string start_location = current_token().get_location();
    print_info("start parsing block statement", MessageType::parser_debug);
    
    // determine the block type, i.e. curly-brace or indent
    // must trust the assertion above that other types of tokens won't end up here
    TokenType ending_token;

    switch (current_token().type) {
        case TokenType::curly_brace_open:
            ending_token = TokenType::curly_brace_close;
            break;
        case TokenType::indent_block_start:
            ending_token = TokenType::indent_block_end;
            break;
        default:
            assert(false && "unhandled block started in parse_block_statement");
    }

    get_token(); // eat start token

    AST::Statement* statement = ast_->create_statement(AST::BlockStatement{});
    // fetch the substatements vector
    std::vector<AST::Statement*>* substatements = &std::get<AST::BlockStatement>(*statement).statements;

    while (
        current_token().type != ending_token            ||
            indent_level_ > indent_at_start             || 
            curly_brace_level_ > curly_brace_at_start
    ) {
        if (current_token().type == TokenType::eof) {
            print_error("Unexpected eof while parsing block statement");
            return statement;
        }

        AST::Statement* substatement = parse_statement();
        // discard empty statements
        if (!std::holds_alternative<AST::EmptyStatement>(*substatement))
            substatements->push_back(substatement);
    }

    get_token(); // eat block closer
    
    if (current_token().type == TokenType::semicolon)
        get_token(); // eat possible trailing semicolon

    print_info("finished parsing block statement from " + start_location, MessageType::parser_debug);
    return statement;
}

AST::Statement* Parser::parse_return_statement() {
    assert(current_token().type == TokenType::reserved_word && current_token().value == "return" 
        && "parse_return_statement called with current token other than \"return\"");

    get_token(); //eat return
    AST::Expression* expression = parse_expression();
    AST::Statement* statement = ast_->create_statement(AST::ReturnStatement{expression});

    assert(is_statement_separator(current_token()) && "return statement didn't end in statement separator");
    get_token(); //eat statement separator

    print_info("parsed return statement", MessageType::parser_debug);
    return statement;
}

// --------- EXPRESSIONS --------

// how to signal failure
AST::Expression* Parser::parse_expression() {

    switch (current_token().type) {
        case TokenType::eof:
            return ast_->create_expression(AST::ExpressionType::syntax_error);

        case TokenType::identifier: {
            Token next_token = peek();

            if (is_statement_separator(next_token))
                return handle_identifier();

            if (is_access_operator(next_token))
                return parse_termed_expression();

            switch (next_token.type) {
                case TokenType::identifier:
                case TokenType::operator_t:
                case TokenType::number:
                case TokenType::string_literal:
                case TokenType::tie:
                    return parse_termed_expression();    

                case TokenType::eof:
                case TokenType::semicolon:
                case TokenType::indent_block_end:
                    return handle_identifier();

                default:
                    return handle_identifier();
            }
        }

        case TokenType::operator_t:
            return parse_termed_expression();

        case TokenType::number: 
            if (is_term_token(peek()))
                return parse_termed_expression();
            return handle_numeric_literal();

        case TokenType::string_literal: 
            if (is_term_token(peek()))
                return parse_termed_expression();
            return handle_string_literal();
            
        // these guys are not LL(1), so we need to always assume they are termed
        case TokenType::parenthesis_open:
        case TokenType::bracket_open:
        case TokenType::curly_brace_open:
            return parse_termed_expression();

        case TokenType::indent_block_start: {
            unsigned int starting_indent_level = indent_level_;
            get_token(); // eat the starting indent
            AST::Expression* expression = parse_expression();
            
            if (current_token().type != TokenType::indent_block_end) {
                print_error("Mismatched indents!");
                return expression;
            }

            get_token(); // eat the ending indent
            assert(indent_level_ <= starting_indent_level && "Mismatched indents");
            return expression;
        }

        case TokenType::reserved_word:
            if (current_token().value != "let") {
                print_error("unknown " + current_token().get_str());
                get_token();
                return ast_->create_expression(AST::ExpressionType::syntax_error);
            }
            
            print_error("Scoped let not yet implemented");
            get_token();
            return ast_->create_expression(AST::ExpressionType::not_implemented);
            
        default:
            print_error("unexpected " + current_token().get_str() + ", at the start of an expression");
            get_token();
    }

    return ast_->create_expression(AST::ExpressionType::string_literal);
}

// expects to be called with the first term as current
AST::Expression* Parser::parse_termed_expression() {
    AST::Expression* expression = ast_->create_expression(AST::ExpressionType::termed_expression);

    expression->terms.push_back(parse_term());

    std::string start_location = current_token().get_location();
    print_info("start parsing termed expression", MessageType::parser_debug);

    bool done = false;
    while (!done) {
        switch (current_token().type) {
            case TokenType::eof:
            case TokenType::indent_block_end:
            case TokenType::semicolon:
            case TokenType::parenthesis_close:
            case TokenType::bracket_close:
            case TokenType::curly_brace_close:
                done = true;
                break;

            case TokenType::parenthesis_open:
            case TokenType::bracket_open:
            case TokenType::curly_brace_open:
                expression->terms.push_back(parse_term());
                continue;

            case TokenType::string_literal:
            case TokenType::number:
            case TokenType::identifier:
            case TokenType::operator_t:
            case TokenType::indent_block_start:
                expression->terms.push_back(parse_term());
                continue;

            default:
                print_error("unexpected: " + current_token().get_str() + ", in termed expression");
                expression->terms.push_back(ast_->create_expression(AST::ExpressionType::not_implemented));
                get_token();
        }
    }

    print_info("finished parsing termed expression from " + start_location, MessageType::parser_debug);
    return expression;

}

AST::Expression* Parser::parse_access_expression() {
    // TODO: eat until the closing character
    get_token();
    return ast_->create_expression(AST::ExpressionType::not_implemented);
}

// expects to be called with the opening parenthese as the current_token_
AST::Expression* Parser::parse_parenthesized_expression() {
    get_token(); // eat '('

    if (current_token().type == TokenType::parenthesis_close) {
        print_error("Empty parentheses in an expression");
        get_token();
        return ast_->create_expression(AST::ExpressionType::syntax_error);
    }

    AST::Expression* expression = parse_expression();
    if (current_token().type != TokenType::parenthesis_close) {
        print_error("Mismatched parentheses");
        return expression;
    }

    get_token(); // eat ')'
    return expression;
}

AST::Expression* Parser::parse_mapping_literal() {
    get_token();
    // eat until the closing character

    print_info("parsed mapping literal (not implemented)", MessageType::parser_debug);

    return ast_->create_expression(AST::ExpressionType::not_implemented);
}

AST::Expression* Parser::handle_string_literal() {
    auto expr = ast_->create_expression(AST::ExpressionType::string_literal);
    expr->string_value = current_token().value;

    get_token();
    get_token(); // eat closing '"'
    
    print_info("parsed string literal", MessageType::parser_debug_terminals);
    return expr;
}

AST::Expression* Parser::handle_numeric_literal() {
    auto expression = ast_->create_expression(AST::ExpressionType::numeric_literal, &AST::NumberLiteral);
    expression->string_value = current_token().value;
    
    get_token();

    print_info("parsed numeric literal", MessageType::parser_debug_terminals);
    return expression;
}

AST::Expression* Parser::handle_identifier() {
    AST::Expression* expression = ast_->create_expression(AST::ExpressionType::unresolved_identifier);
    print_info("parsed unresolved identifier", MessageType::parser_debug_terminals);

    expression->string_value = current_token().value;

    get_token();
    return expression;
}

// Expects not to be called if the current token is not parseable into a term
AST::Expression* Parser::parse_term() {
    switch (current_token().type) {
        case TokenType::identifier:
            // TODO: revamp access expressions
            return handle_identifier();
        case TokenType::string_literal:
            return handle_string_literal();
        case TokenType::number:
            return handle_numeric_literal();

        case TokenType::tie:
            get_token();
            return ast_->create_expression(AST::ExpressionType::tie);

        case TokenType::parenthesis_open: 
            return parse_parenthesized_expression();

        case TokenType::bracket_open:
        case TokenType::curly_brace_open:
            return parse_mapping_literal();
        
        case TokenType::indent_block_start:
            return parse_expression();
            
        case TokenType::operator_t: {
            AST::Expression* expression = ast_->create_expression(AST::ExpressionType::unresolved_operator);
            expression->string_value = current_token().value;
            get_token();
            return expression;
        }

        default:
            assert(false && "Parser::parse_term called with a non-term token");
    }
}

// AST::Expression* Parser::parse_call_expression(const std::string& callee) {
//     // TODO: handle tuple arg lists
//     assert(
//         (current_token().type == TokenType::identifier)
//         && "Parser::parse_call_expression(Token) called when the current_token() was not an identifier"
//     );

//     // check that the function exists here, or mark as hanging
//     auto call = ast_->create_expression(AST::ExpressionType::call);

//     // parse the following tokens as an argument list
//     call->call_expr = {callee, parse_argument_list()};
//     return call;
// }

// AST::Expression* Parser::parse_call_expression(AST::Expression* callee, const std::vector<AST::Type*>& signature) {
//     assert(callee && "Passed nullptr to Parser::parse_call_expression");

//     if (signature.size() > 0)
//         print_error("Parser::parse_call_expression doesn't know how to handle type signatures yet, ignoring the signature");
    
//     switch (callee->expression_type) {
//         case AST::ExpressionType::unresolved_identifier:
//             return parse_call_expression(callee->string_value);

//         case AST::ExpressionType::native_function:
//         case AST::ExpressionType::native_operator: { // why the heck not
        
//             auto maybe_callee = lookup_identifier(current_token().value);
//             // TODO: ideally assert here that the callee exists as a builtin
//             auto [callee_identifier, _2,  arg_types] = **maybe_callee;
//             unsigned int arity = arg_types.size();
            
//             if (arity == 0) {
//                 auto expr = ast_->create_expression(AST::ExpressionType::call);
//                 expr->call_expr = {current_token().value, {}};
//                 return expr;
//             }
            
//             if (arity != 1) {
//                 print_error("CAN'T YET PARSE ARG LISTS LONGER THAN 1");
//                 declare_invalid();
//                 return ast_->create_expression(AST::ExpressionType::call);
//             }
            
//             // !!! this arg stuff is something to be deferred, since we might later get polymorphic defs with different arity
//             // !!! On the other hand, with built ins and such we already know the arity
//             // Therefore, builtins are different semantically?
//             // We must make sure that whether something is a builtin or not does not affect semantics
//             // Yes, with inference we must concretize types as soon as we can to reduce the complexity

//             AST::Expression* arg = parse_expression();

//             auto expr = ast_->create_expression(AST::ExpressionType::call);
//             expr->call_expr = {callee_identifier, {arg}};
//             return expr;
//         }
//         default:
//             assert(false && "Parser::parse_call_expression called with an expression that's not callable");
//     }
// }

// AST::Expression* Parser::parse_operator_expression(Token previous_token) {
//     return ast_->create_expression(AST::ExpressionType::not_implemented);
// }

// // TODO: make arg lists an expressiontype
// std::vector<AST::Expression*> Parser::parse_argument_list() {
//     assert(false && "Not ready");
//     return {};    
// }

void Parser::reset_to_top_level() {
    print_info("resetting to global scope", MessageType::parser_debug);

    while (
        current_token().type != TokenType::eof          && (
            !is_statement_separator(current_token())    || 
            angle_bracket_level_    >   0               ||
            curly_brace_level_      >   0               ||
            parenthese_level_       >   0               ||
            indent_level_           >   0
        )
    ) get_token();
    get_token();
}
