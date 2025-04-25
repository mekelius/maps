/**
 * Recursive descent parser for maps programming language
 * 
 * The convention here (unlike in the lexer) is that every production rule must move the buffer
 * beyond the tokens it consumed.
 * 
 * Same if a token is rejected. 
 */

#include <variant>
#include <cassert>

#include "../logging.hh"
#include "../source.hh"

#include "../lang/builtins.hh"

#include "token.hh"
#include "parser_layer1.hh"

using Logging::LogLevel;
using Logging::MessageType;

// ----- PUBLIC METHODS -----

ParserLayer1::ParserLayer1(StreamingLexer* lexer, Pragma::Pragmas* pragmas, bool in_repl):
lexer_(lexer), pragmas_(pragmas), in_repl_(in_repl) {
    ast_ = std::make_unique<Maps::AST>();
    get_token();
    get_token();
}

std::unique_ptr<Maps::AST> ParserLayer1::run() {    
    init_builtins(*ast_);
    Maps::Statement* root = ast_->create_statement(Maps::StatementType::block, {0,0});
    ast_->set_root(root);

    while (current_token().token_type != TokenType::eof) {
        int prev_buf_slot = which_buf_slot_; // a bit of a hack, an easy way to do the assertion below

        parse_top_level_statement();

        assert(which_buf_slot_ != prev_buf_slot && 
            "Parser::parse_top_level_statement didn't advance the tokenstream");
    }

    return std::move(ast_);
}
// ----- PRIVATE METHODS -----

Token ParserLayer1::get_token() {
    token_buf_[which_buf_slot_ % 2] = lexer_->get_token();
    which_buf_slot_++;

    // manage the parentheses and indents etc.
    update_brace_levels(current_token());

    return current_token();
}

Token ParserLayer1::current_token() const {
    return token_buf_[which_buf_slot_ % 2];
}

Token ParserLayer1::peek() const {
    return token_buf_[(which_buf_slot_+1) % 2];
}

void ParserLayer1::update_brace_levels(Token token) {
    switch (token.token_type) {
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

void ParserLayer1::declare_invalid() {
    ast_->is_valid = false;
}


// ----- OUTPUT HELPERS -----

void ParserLayer1::log_error(const std::string& message) const {
    log_error(current_token().location, message);
}

void ParserLayer1::log_error(SourceLocation location, const std::string& message) const {
    Logging::log_error(location, message);
}

void ParserLayer1::log_info(const std::string& message, Logging::MessageType message_type) const {
    log_info(current_token().location, message, message_type);
}

void ParserLayer1::log_info(SourceLocation location, const std::string& message, Logging::MessageType message_type) const {
    Logging::log_info(location, message, message_type);
}


// ----- IDENTIFIERS -----

bool ParserLayer1::identifier_exists(const std::string& identifier) const {
    return ast_->globals_->identifier_exists(identifier);
}

void ParserLayer1::create_identifier(const std::string& name, SourceLocation location) {
    create_identifier(name, std::monostate{}, location);
}

void ParserLayer1::create_identifier(const std::string& name,
    Maps::CallableBody body, SourceLocation location) {
    log_info("created identifier" + name, Logging::MessageType::parser_debug_identifier);
    ast_->globals_->create_callable(name, body, location);
}

std::optional<Maps::Callable*> ParserLayer1::lookup_identifier(const std::string& identifier) {
    return ast_->globals_->get_identifier(identifier);
}


// ----- EXPRESSION AND STATEMENT HELPERS -----

void ParserLayer1::expression_start() {
    current_expression_start_.push_back(current_token().location);
}

void ParserLayer1::statement_start() {
    current_statement_start_.push_back(current_token().location);
}

// creates an expression using ast_, marking the location as the current_expression_start_
// or if it's not set, the current token location
void ParserLayer1::expression_end() {
    current_expression_start_.pop_back();
}

// creates a statement using ast_, marking the location as the current_statement_start_
// or if it's not set, the current token location
Maps::Statement* ParserLayer1::create_statement(Maps::StatementType statement_type) {
    if (current_statement_start_.empty())
        return ast_->create_statement(statement_type, current_token().location);

    Maps::Statement* statement = ast_->create_statement(statement_type, current_statement_start_.back());
    current_statement_start_.pop_back();
    return statement;
}


// ########## PARSING ##########

// NOTE: pragma.cpp does its own logging
void ParserLayer1::handle_pragma() {
    // get the first word
    std::istringstream token_value_iss{current_token().string_value()};
    std::string value_string;
    std::getline(token_value_iss, value_string, ' ');

    std::string flag_name;
    std::getline(token_value_iss, flag_name);
    
    // convert the forst word into bool
    bool value;
    if (value_string == "enable") {
        value = true;
    } else if (value_string == "disable") {
        value = false;
    } else {
        log_error("invalid pragma declaration");
        declare_invalid();
        get_token();
        return;
    }

    // the rest should be the flag name
    bool succeeded = pragmas_->set_flag(
        flag_name, value, current_token().location);
    
    if (!succeeded) 
        declare_invalid();

    get_token();
}


// --------- STATEMENTS --------

Maps::Statement* ParserLayer1::broken_statement_helper(const std::string& message) {
    log_error(message);
    declare_invalid();
    Maps::Statement* statement = create_statement(Maps::StatementType::broken);
    reset_to_top_level();
    return statement;
}

void ParserLayer1::parse_top_level_statement() {
    Maps::Statement* statement;

    switch (current_token().token_type) {
        case TokenType::pragma:
            handle_pragma();
            return parse_top_level_statement();

        case TokenType::eof:
            finished_ = true;
            return;

        default:
            // TODO: check pragmas for top-level statement types
            statement = parse_statement();
            if (statement->statement_type != Maps::StatementType::empty && 
                (pragmas_->check_flag_value("top-level evaluation", statement->location) || in_repl_)) {            
                std::get<Maps::Block>(std::get<Maps::Statement*>(ast_->root_->body)->value).push_back(statement);
            }
            return;
    }
}

Maps::Statement* ParserLayer1::parse_non_global_statement() {
    switch (current_token().token_type) {
        case TokenType::pragma:
            declare_invalid();
            log_error("unexpected non top-level pragma");
            reset_to_top_level();
            return create_statement(Maps::StatementType::broken); 

        case TokenType::eof:
            finished_ = true;
            return create_statement(Maps::StatementType::broken);

        default:
            return parse_statement();
    }
}

Maps::Statement* ParserLayer1::parse_statement() {
    switch (current_token().token_type) {
        case TokenType::eof:
            finished_ = true;
            return create_statement(Maps::StatementType::empty);

        case TokenType::reserved_word:
            log_info("scoping not yet implemented");
            if (current_token().string_value() == "let")
                return parse_let_statement();

            if (current_token().string_value() == "operator")
                return parse_operator_statement();
            
            assert(false && "Unhandled reserved word in Parser::parse_top_level_statement");
            
        case TokenType::identifier:
            if (peek().token_type == TokenType::operator_t && peek().string_value() == "=") {
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
            return create_statement(Maps::StatementType::empty);

        // ----- ignored -----
        case TokenType::dummy:
            get_token(); // eat the dummy
            return create_statement(Maps::StatementType::empty);
            
        // ---- errors -----
        default:
            declare_invalid();    
            log_error("use \"@ enable top level context\" to allow top level evaluation");
            reset_to_top_level();
            return create_statement(Maps::StatementType::broken);
        
        case TokenType::indent_block_end:
            assert(false && "parse_statement called at indent_block_end");
            return create_statement(Maps::StatementType::broken);
        
        case TokenType::indent_error_fatal:
            declare_invalid();
            log_error("indent error");
            reset_to_top_level();
            return create_statement(Maps::StatementType::broken);

        case TokenType::unknown:
            declare_invalid();
            log_error("unknown token");
            reset_to_top_level();
            return create_statement(Maps::StatementType::broken);
    }
}

Maps::Statement* ParserLayer1::parse_expression_statement() {
    Maps::Expression* expression = parse_expression();
    Maps::Statement* statement = create_statement(Maps::StatementType::expression_statement);
    statement->value = expression;

    assert(is_block_starter(current_token()) || is_statement_separator(current_token()) && "statement didn't end in a statement separator");

    get_token(); // eat statement separator
    log_info("finished parsing expression statement from " + statement->location.to_string(), MessageType::parser_debug);
    return statement;
}

Maps::Statement* ParserLayer1::parse_let_statement() {
    statement_start();
    
    switch (get_token().token_type) {
        case TokenType::identifier:
            // TODO: check lower case here
            {
                std::string name = current_token().string_value();
                
                // check if name already exists
                if (identifier_exists(name)) {
                    declare_invalid();
                    log_error("Attempting to redefine identifier " + name);
                    Maps::Statement* statement = create_statement(Maps::StatementType::illegal);
                    statement->value = "Attempting to redefine identifier: " + name;
                    return statement;
                }

                get_token(); // eat the identifier

                if (is_statement_separator(current_token())) {
                    log_info("parsed let statement declaring \"" + name + "\" with no definition", MessageType::parser_debug);
                    
                    
                    get_token(); // eat the semicolon
                    Maps::Statement* statement = create_statement(Maps::StatementType::let);

                    // create an unitialized identifier
                    create_identifier(name, std::monostate{}, statement->location);
                    statement->value = Maps::Let{ name , std::monostate{} };

                    return statement;
                }

                if (is_assignment_operator(current_token())) {
                    get_token(); // eat the assignment operator

                    Maps::CallableBody body;
                    if (is_block_starter(current_token())) {
                        body = parse_block_statement();
                    } else {
                        body = parse_expression();
                    }

                    Maps::Statement* statement = create_statement(Maps::StatementType::let);
                    statement->value = Maps::Let{name, body};

                    create_identifier(name, body, statement->location);
                    log_info("parsed let statement", MessageType::parser_debug);
                    return statement;
                }

                get_token();
                declare_invalid();
                log_error("unexpected " + current_token().get_string() + ", in let-statement");
                reset_to_top_level();
                return create_statement(Maps::StatementType::broken);
            }

        case TokenType::operator_t:
            log_info("operator overloading not yet implemented, ignoring");
            reset_to_top_level();
            return create_statement(Maps::StatementType::empty);

        default:
            declare_invalid();
            log_error("unexpected token: " + current_token().get_string() + " in let statement");
            reset_to_top_level();
            return create_statement(Maps::StatementType::broken);
    }
}

Maps::Statement* ParserLayer1::parse_operator_statement() {
    statement_start();

    switch (get_token().token_type) {
        case TokenType::operator_t: {
            std::string op = current_token().string_value();

            if (ast_->builtins_scope_->identifier_exists(op)) 
                return broken_statement_helper("attempting to redefine built-in operator: " + op);

            if (ast_->globals_->identifier_exists(op))
                return broken_statement_helper("attempting to redefine user-defined operator: " + op);

            get_token();

            if (!is_assignment_operator(current_token()))
                return broken_statement_helper(
                    "unexpected token: " + current_token().get_string() + " in operator statement, expected \"=\"");

            get_token();

            if (current_token().token_type != TokenType::reserved_word || (current_token().string_value() != "unary" && current_token().string_value() != "binary"))
                    return broken_statement_helper(
                        "unexpected token: " + current_token().get_string() + " in operator statement, expected \"unary|binary\"");

            unsigned int arity = current_token().string_value() == "binary" ? 2 : 1;

            get_token();

            // UNARY OPERATOR
            if (arity == 1) {
                if (current_token().token_type != TokenType::reserved_word || (current_token().string_value() != "prefix" && current_token().string_value() != "postfix"))
                    return broken_statement_helper("unexpected token: " + current_token().get_string() + " in unary operator statement, expected \"prefix|postfix\"");

                assert(current_token().get_string() == "prefix" && "postfix operators not implemented");
                // Maps::UnaryFixity fixity = current_token().get_string() == "prefix" ? Maps::UnaryFixity::prefix : Maps::UnaryFixity::postfix;
                Maps::UnaryFixity fixity = Maps::UnaryFixity::prefix;

                get_token(); // eat the fixity specifier

                Maps::CallableBody body;
                if (is_block_starter(current_token())) {
                    body = parse_block_statement();
                } else {
                    body = parse_expression();
                }

                Maps::Statement* statement = create_statement(Maps::StatementType::operator_s);
                statement->value = Maps::OperatorStatementValue{op, 1, body};

                ast_->globals_->create_unary_operator(op, body, fixity, statement->location);
                log_info("parsed let statement", MessageType::parser_debug);
                return statement;   
            }

            // BINARY OPERATOR
            if (current_token().token_type != TokenType::number)
                return broken_statement_helper("unexpected token: " + current_token().get_string() + " in unary operator statement, expected precedence specifier(positive integer)");

            unsigned int precedence = std::stoi(current_token().string_value());
            if (precedence >= Maps::MAX_OPERATOR_PRECEDENCE)
                return broken_statement_helper("max operator precedence is " + std::to_string(Maps::MAX_OPERATOR_PRECEDENCE));

            get_token(); // eat the precedence specifier

            Maps::CallableBody body;
            if (is_block_starter(current_token())) {
                body = parse_block_statement();
            } else {
                body = parse_expression();
            }

            Maps::Statement* statement = create_statement(Maps::StatementType::operator_s);
            statement->value = Maps::OperatorStatementValue{op, 2, body};

            ast_->globals_->create_binary_operator(op, body, precedence, Maps::Associativity::left, statement->location);
            log_info("parsed let statement", MessageType::parser_debug);
            return statement;   

        }
        default:
            return broken_statement_helper("unexpected token: " + current_token().get_string() + " in operator statement");            
    }
}

Maps::Statement* ParserLayer1::parse_assignment_statement() {
    assert(current_token().token_type == TokenType::identifier 
        && "parse_assignment_statement called with current_token that was not an identifier");

    statement_start();
    std::string name = current_token().string_value();

    get_token();
    // TODO: change '=' to it's own TokenType
    // !!! in any case assignment operators should live in their own category
    assert(is_assignment_operator(current_token())
        && "assignment statement second token not an assignment operator");
   
    get_token(); // eat '='

    Maps::Statement* inner_statement = parse_statement();
    Maps::Statement* statement = create_statement(Maps::StatementType::assignment);
    statement->value = Maps::Assignment{name, inner_statement};

    // expect parse_statement to eat the semicolon etc.

    log_info("finished parsing assignment statement from " + statement->location.to_string(), MessageType::parser_debug);
    return statement;
}

Maps::Statement* ParserLayer1::parse_block_statement() {
    assert(is_block_starter(current_token())
        && "parse_block_statement called with current token that is not a block starter");

    unsigned int indent_at_start = indent_level_;
    unsigned int curly_brace_at_start = curly_brace_level_;

    statement_start();
    log_info("start parsing block statement", MessageType::parser_debug);
    
    // determine the block type, i.e. curly-brace or indent
    // must trust the assertion above that other types of tokens won't end up here
    TokenType ending_token;

    switch (current_token().token_type) {
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

    Maps::Statement* statement = create_statement(Maps::StatementType::block);
    // fetch the substatements vector
    std::vector<Maps::Statement*>* substatements = &std::get<Maps::Block>(statement->value);

    while (
        current_token().token_type != ending_token            ||
            indent_level_ > indent_at_start             || 
            curly_brace_level_ > curly_brace_at_start
    ) {
        if (current_token().token_type == TokenType::eof) {
            declare_invalid();
            log_error("Unexpected eof while parsing block statement");
            return statement;
        }

        Maps::Statement* substatement = parse_statement();
        // discard empty statements
        if (substatement->statement_type != Maps::StatementType::empty)
            substatements->push_back(substatement);
    }

    get_token(); // eat block closer
    
    if (current_token().token_type == TokenType::semicolon)
        get_token(); // eat possible trailing semicolon

    log_info("finished parsing block statement from " + statement->location.to_string(), 
        MessageType::parser_debug);
    return statement;
}

Maps::Statement* ParserLayer1::parse_return_statement() {
    assert(current_token().token_type == TokenType::reserved_word && current_token().string_value() == "return" 
        && "parse_return_statement called with current token other than \"return\"");

    get_token(); //eat return
    Maps::Expression* expression = parse_expression();
    Maps::Statement* statement = create_statement(Maps::StatementType::return_);
    statement->value = expression;

    assert(is_statement_separator(current_token()) 
        && "return statement didn't end in statement separator");
    get_token(); //eat statement separator

    log_info("parsed return statement", MessageType::parser_debug);
    return statement;
}

// --------- EXPRESSIONS --------

Maps::Expression* ParserLayer1::parse_expression() {
    switch (current_token().token_type) {
        case TokenType::eof: {
            Maps::Expression* expression = ast_->create_valueless_expression(
                Maps::ExpressionType::syntax_error, current_expression_start_.back());
            expression_end();
            return expression;
        }

        case TokenType::identifier: {
            Token next_token = peek();

            if (is_statement_separator(next_token))
                return handle_identifier();

            if (is_access_operator(next_token))
                return parse_termed_expression();

            switch (next_token.token_type) {
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
            Maps::Expression* expression = parse_expression();
            
            if (current_token().token_type != TokenType::indent_block_end) {
                declare_invalid();
                log_error("Mismatched indents!");
                return expression;
            }

            get_token(); // eat the ending indent
            assert(indent_level_ <= starting_indent_level && "Mismatched indents");
            return expression;
        }

        case TokenType::reserved_word: {
            if (current_token().string_value() != "let") {
                declare_invalid();
                log_error("unknown " + current_token().get_string());
                Maps::Expression* expression = ast_->create_valueless_expression(
                    Maps::ExpressionType::syntax_error, current_expression_start_.back());
                expression_end();
                expression->value = "unknown " + current_token().get_string();
                
                get_token();
                return expression;
            }
            declare_invalid();
            log_error("Scoped let not yet implemented");
            Maps::Expression* expression = ast_->create_valueless_expression(
                Maps::ExpressionType::syntax_error, current_expression_start_.back());
            expression_end();
            expression->value = "Scoped let not yet implemented";
            
            get_token();
            return expression;
        }
            
        default:
            declare_invalid();
            log_error("unexpected " + current_token().get_string() + ", at the start of an expression");
            Maps::Expression* expression = ast_->create_valueless_expression(Maps::ExpressionType::syntax_error, current_expression_start_.back());
            expression_end();
            expression->value = "unexpected " + current_token().get_string() + ", at the start of an expression";

            get_token();
            return expression;
    }
}

// expects to be called with the first term as current
// tied expression = no whitespace
Maps::Expression* ParserLayer1::parse_termed_expression(bool in_tied_expression) {
    expression_start();
    Maps::Expression* expression = ast_->create_termed_expression({}, current_token().location);

    log_info("start parsing termed expression", MessageType::parser_debug);

    expression->terms().push_back(parse_term(in_tied_expression));

    bool done = false;
    while (!done) {
        // in tied expressions, every term must be followed by a tie
        // if not, we are done
        if(in_tied_expression) {
            if (current_token().token_type != TokenType::tie) {
                done = true;
                break;
            } else {
                get_token(); // eat the tie
            }
        }

        switch (current_token().token_type) {
            // ties create a new sub-expression, unless we are already in one
            case TokenType::tie:
                assert(false && "somehow a tie leaked into the termed expression loop");
                get_token();
                break;

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
                expression->terms().push_back(parse_term(in_tied_expression));
                break;

            case TokenType::string_literal:
            case TokenType::number:
            case TokenType::identifier:
            case TokenType::operator_t:
            case TokenType::indent_block_start:
                expression->terms().push_back(parse_term(in_tied_expression));
                break;

            default:
                declare_invalid();
                log_error("unexpected: " + current_token().get_string() + ", in termed expression");
                Maps::Expression* term = ast_->create_valueless_expression(
                    Maps::ExpressionType::not_implemented, current_token().location);
                term->value = current_token().get_string();
                expression->terms().push_back(term);
                
                get_token();
        }
    }
    ast_->unparsed_termed_expressions.push_back(expression);

    log_info("finished parsing termed expression from " + expression->location.to_string(), MessageType::parser_debug);
    expression_end();
    return expression;
}

Maps::Expression* ParserLayer1::parse_access_expression() {
    // TODO: eat until the closing character
    expression_start();
    Maps::Expression* expression = ast_->create_valueless_expression(
        Maps::ExpressionType::not_implemented, current_token().location);
    get_token();
    expression_end();
    return expression;
}

// expects to be called with the opening parenthese as the current_token_
Maps::Expression* ParserLayer1::parse_parenthesized_expression() {
    expression_start();
    get_token(); // eat '('

    if (current_token().token_type == TokenType::parenthesis_close) {
        declare_invalid();
        log_error("Empty parentheses in an expression");
        Maps::Expression* expression = ast_->create_valueless_expression(Maps::ExpressionType::syntax_error, current_token().location);
        get_token();
        expression_end();
        return expression;
    }

    Maps::Expression* expression = parse_expression();
    if (current_token().token_type != TokenType::parenthesis_close) {
        declare_invalid();
        log_error("Mismatched parentheses");
        return expression;
    }

    get_token(); // eat ')'
    return expression;
}

Maps::Expression* ParserLayer1::parse_mapping_literal() {
    expression_start();
    Maps::Expression* expression = ast_->create_valueless_expression(Maps::ExpressionType::not_implemented, 
        current_token().location);
    get_token();
    // eat until the closing character

    log_info("parsed mapping literal (not implemented)", MessageType::parser_debug);

    expression_end();
    return expression;
}

Maps::Expression* ParserLayer1::handle_string_literal() {
    Maps::Expression* expression = ast_->create_string_literal(current_token().string_value(), 
        current_token().location);

    get_token();
    get_token(); // eat closing '"'
    
    log_info("parsed string literal", MessageType::parser_debug_terminal);
    return expression;
}

Maps::Expression* ParserLayer1::handle_numeric_literal() {
    Maps::Expression* expression = ast_->create_numeric_literal(current_token().string_value(), 
        current_token().location);
    
    get_token();
        
    log_info("parsed numeric literal", MessageType::parser_debug_terminal);
    return expression;
}

Maps::Expression* ParserLayer1::handle_identifier() {
    Maps::Expression* expression = ast_->create_identifier_expression(current_token().string_value(), 
        current_token().location);
   
    get_token();
    
    log_info("parsed unresolved identifier", MessageType::parser_debug_terminal);
    return expression;
}

// Expects not to be called if the current token is not parseable into a term
Maps::Expression* ParserLayer1::parse_term(bool is_tied) {
    // if we see a tie, go down into a tied expression if not already in one
    if (peek().token_type == TokenType::tie && !is_tied) {
        return parse_termed_expression(true);
    }

    switch (current_token().token_type) {
        case TokenType::identifier:
            // TODO: revamp access expressions
            return handle_identifier();
        case TokenType::string_literal:
            return handle_string_literal();
        case TokenType::number:
            return handle_numeric_literal();

        case TokenType::tie: {
            Maps::Expression* expression = ast_->create_valueless_expression(Maps::ExpressionType::tie, 
                current_token().location);
            get_token();
            return expression;   
        }

        case TokenType::parenthesis_open: 
            return parse_parenthesized_expression();

        case TokenType::bracket_open:
        case TokenType::curly_brace_open:
            return parse_mapping_literal();
        
        case TokenType::indent_block_start:
            return parse_expression();
            
        case TokenType::operator_t: {
            Maps::Expression* expression = ast_->create_operator_expression(current_token().string_value(), 
                current_token().location);
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
        
//             auto maybe_callee = lookup_identifier(current_token().string_value());
//             // TODO: ideally assert here that the callee exists as a builtin
//             auto [callee_identifier, _2,  arg_types] = **maybe_callee;
//             unsigned int arity = arg_types.size();
            
//             if (arity == 0) {
//                 auto expr = ast_->create_expression(AST::ExpressionType::call);
//                 expr->call_expr = {current_token().string_value(), {}};
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

void ParserLayer1::reset_to_top_level() {
    log_info("resetting to global scope", MessageType::parser_debug);

    while (
        current_token().token_type != TokenType::eof          && (
            !is_statement_separator(current_token())    || 
            angle_bracket_level_    >   0               ||
            curly_brace_level_      >   0               ||
            parenthese_level_       >   0               ||
            indent_level_           >   0
        )
    ) get_token();
    get_token();
}
