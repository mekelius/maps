/**
 * Recursive descent parser for maps programming language
 * 
 * The convention here (unlike in the lexer) is that every production rule must move the buffer
 * beyond the tokens it consumed.
 * 
 * Same if a token is rejected. 
 */
#include "parser_layer1.hh"

#include <cassert>
#include <initializer_list>
#include <sstream>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/pragma.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/types/type.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/operator.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/ast_store.hh"

#include "mapsc/parser/token.hh"
#include "mapsc/procedures/simplify.hh"

using std::optional, std::nullopt, std::make_unique;


namespace Maps {

using Log = LogInContext<LogContext::layer1>;

// ----- PUBLIC METHODS -----

ParserLayer1::ParserLayer1(CompilationState* const compilation_state)
:compilation_state_(compilation_state), 
 ast_store_(compilation_state->ast_store_.get()), 
 pragmas_(&compilation_state->pragmas_) {}

bool ParserLayer1::run(std::istream& source_is) {
    run_parse(source_is);
    return compilation_state_->is_valid;
}

optional<Definition*> ParserLayer1::eval_parse(std::istream& source_is) {
    force_top_level_eval_ = true;
    run_parse(source_is);
    force_top_level_eval_ = false;

    if (!compilation_state_->entry_point_) {
        fail("No entry point");
        return nullopt;
    }

    auto entry_point = *compilation_state_->entry_point_;
    // if root is a single statement block or an expression statement, simplify it
    attempt_simplify(*entry_point);

    // check for empty entry point
    if (entry_point->is_empty())
        compilation_state_->entry_point_ = nullopt;

    return compilation_state_->entry_point_;
}

// ----- PRIVATE METHODS -----

void ParserLayer1::run_parse(std::istream& source_is) {
    lexer_ = make_unique<Lexer>(&source_is);

    prime_tokens();

    Statement* root_statement = ast_store_->allocate_statement({StatementType::block, {0,0}});
    RT_Definition* root_definition = ast_store_->allocate_definition(RT_Definition{
        "root", root_statement, {0,0}});

    if (!compilation_state_->set_entry_point(root_definition))
        return fail("failed to set entry point");

    while (current_token().token_type != TokenType::eof) {
        #ifndef NDEBUG
        int prev_buf_slot = which_buf_slot_; // a bit of a hack, an easy way to do the assertion below
        #endif

        parse_top_level_chunk();

        assert(which_buf_slot_ != prev_buf_slot && 
            "Parser::parse_top_level_statement didn't advance the tokenstream");
    }

    lexer_ = nullptr;
}

void ParserLayer1::prime_tokens() {
    get_token();
    get_token();
}

Token ParserLayer1::get_token() {
    token_buf_[which_buf_slot_ % 2] = lexer_->get_token();
    which_buf_slot_++;

    // manage the parentheses and indents etc.
    update_brace_levels(current_token());

    return current_token();
}

const Token& ParserLayer1::current_token() const {
    return token_buf_[which_buf_slot_ % 2];
}

const Token& ParserLayer1::peek() const {
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
    compilation_state_->is_valid = false;
}


// ----- OUTPUT HELPERS -----

void ParserLayer1::fail(const std::string& message, SourceLocation location) {
    Log::error(message, location);
    declare_invalid();
}
void ParserLayer1::fail(const std::string& message) {
    fail(message, current_token().location);
}

Expression* ParserLayer1::fail_expression(const std::string& message, SourceLocation location) {
    fail(message, location);
    Expression* expression = Expression::syntax_error(*ast_store_, location);
    get_token();
    return expression;
}
Expression* ParserLayer1::fail_expression(const std::string& message) {
    return fail_expression(message, current_token().location);
}
    

void ParserLayer1::log(const std::string& message, LogLevel loglevel) const {
    Log::on_level(message, loglevel, current_token().location);
}

void ParserLayer1::log(const std::string& message, LogLevel loglevel, 
    SourceLocation location) const {
    
    Log::on_level(message, loglevel, location);
}


// ----- IDENTIFIERS -----

bool ParserLayer1::identifier_exists(const std::string& identifier) const {
    return compilation_state_->globals_.identifier_exists(identifier);
}

void ParserLayer1::create_identifier(const std::string& name, SourceLocation location) {
    create_identifier(name, Undefined{}, location);
}

void ParserLayer1::create_identifier(const std::string& name,
    DefinitionBody body, SourceLocation location) {
    log("created identifier " + name, LogLevel::debug_extra);
    compilation_state_->globals_.create_identifier(
        ast_store_->allocate_definition(RT_Definition{name, body, location})
    );
}

std::optional<Definition*> ParserLayer1::lookup_identifier(const std::string& identifier) {
    return compilation_state_->globals_.get_identifier(identifier);
}


// ----- EXPRESSION AND STATEMENT HELPERS -----

Definition* ParserLayer1::create_definition(DefinitionBody body, SourceLocation location) {
    return ast_store_->allocate_definition(RT_Definition{body, location});
}

Statement* ParserLayer1::create_statement(StatementType statement_type, SourceLocation location) {
    Statement* statement = ast_store_->allocate_statement({statement_type, location});
    return statement;
}


// #################### PARSING ####################

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
        fail("invalid pragma declaration");
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

Definition* ParserLayer1::broken_definition_helper(const std::string& message, 
    SourceLocation location) {
    
    fail(message);
    Definition* definition = create_definition(UserError{}, location);
    reset_to_top_level();
    return definition;
}

// --------- STATEMENTS --------

Statement* ParserLayer1::broken_statement_helper(const std::string& message, SourceLocation location) {
    fail(message);
    Statement* statement = create_statement(StatementType::broken, location);
    reset_to_top_level();
    return statement;
}

Chunk ParserLayer1::parse_top_level_chunk() {
    Statement* statement;

    switch (current_token().token_type) {
        case TokenType::pragma:
            handle_pragma();
            return parse_top_level_chunk();

        case TokenType::eof:
            finished_ = true;
            return std::monostate{};

        case TokenType::reserved_word:
            if (current_token().string_value() == "let") {
                log("Scoping not yet implemented", LogLevel::warning);
                return parse_let_definition();
            }

            if (current_token().string_value() == "operator")
                return parse_operator_definition();
            
            assert(false && "Unhandled reserved word in Parser::parse_top_level_statement");
            

        default:
            // TODO: check pragmas for top-level statement types
            // Definitions shouldn't be evaluated
            statement = parse_statement();
            if (statement->statement_type != StatementType::empty && !statement->is_definition() &&
                (pragmas_->check_flag_value("top-level evaluation", statement->location) || 
                    force_top_level_eval_)) {            
                std::get<Block>(std::get<Statement*>(
                    (*compilation_state_->entry_point_)->body())->value)
                        .push_back(statement);
            }
            return statement;
    }
}

DefinitionBody ParserLayer1::parse_definition_body() {
    return is_block_starter(current_token()) ?
        DefinitionBody{parse_block_statement()} : DefinitionBody{parse_expression()};
}

Statement* ParserLayer1::parse_non_global_statement() {
    auto location = current_token().location;

    switch (current_token().token_type) {
        case TokenType::pragma:
            fail("unexpected non top-level pragma");
            reset_to_top_level();
            return create_statement(StatementType::broken, location); 

        case TokenType::eof:
            finished_ = true;
            return create_statement(StatementType::broken, location);

        default:
            return parse_statement();
    }
}

Statement* ParserLayer1::parse_statement() {
    auto location = current_token().location;

    switch (current_token().token_type) {
        case TokenType::eof:
            finished_ = true;
            return create_statement(StatementType::empty, location);

        case TokenType::identifier:
            if (peek().token_type == TokenType::operator_t && peek().string_value() == "=") {
                return parse_assignment_statement();
            }
            return parse_expression_statement();

        case TokenType::indent_block_start:
        case TokenType::curly_brace_open:
            return parse_block_statement();

        case TokenType::type_identifier:
        case TokenType::lambda:
        case TokenType::question_mark:
        case TokenType::parenthesis_open:
        case TokenType::bracket_open:
        case TokenType::operator_t:
        case TokenType::string_literal:
        case TokenType::number:
            return parse_expression_statement();

        case TokenType::semicolon:
            get_token(); // eat the semicolon
            return create_statement(StatementType::empty, location);

        // ----- ignored -----
        case TokenType::dummy:
            get_token(); // eat the dummy
            return create_statement(StatementType::empty, location);

        case TokenType::indent_block_end:
            assert(false && "parse_statement called at indent_block_end");
            return create_statement(StatementType::broken, location);
        
        case TokenType::indent_error_fatal:
            fail("indent error");
            reset_to_top_level();
            return create_statement(StatementType::broken, location);

        // ---- errors -----
        default:
            fail("Unexpected "+ current_token().get_string() + ", expected a statement");
            reset_to_top_level();
            return create_statement(StatementType::broken, location);
    }
}

Statement* ParserLayer1::parse_expression_statement() {
    auto location = current_token().location;

    Expression* expression = parse_expression();
    Statement* statement = create_statement(StatementType::expression_statement, location);
    statement->value = expression;

    assert(is_block_starter(current_token()) || is_statement_separator(current_token()) && 
        "statement didn't end in a statement separator");

    if (current_token().token_type == TokenType::semicolon)
        get_token(); // eat trailing semicolon
    log("finished parsing expression statement from " + statement->location.to_string(), 
        LogLevel::debug_extra);
    return statement;
}

Definition* ParserLayer1::parse_let_definition() {
    auto location = current_token().location;
    
    switch (get_token().token_type) {
        case TokenType::identifier:
            // TODO: check lower case here
            {
                std::string name = current_token().string_value();
                
                // check if name already exists
                if (identifier_exists(name)) {
                    fail("Attempting to redefine identifier " + name);
                    Statement* statement = create_statement(StatementType::illegal, location);
                    statement->value = "Attempting to redefine identifier: " + name;
                    return ast_store_->allocate_definition(RT_Definition{UserError{}, 
                        location});
                }

                get_token(); // eat the identifier

                if (is_statement_separator(current_token())) {
                    log("parsed let statement declaring \"" + name + "\" with no definition", 
                        LogLevel::debug_extra);
                    
                    get_token(); // eat the semicolon

                    // create an unitialized identifier
                    create_identifier(name, Undefined{}, location);

                    return ast_store_->allocate_definition(RT_Definition{UserError{}, 
                        location});
                }

                if (is_assignment_operator(current_token())) {
                    get_token(); // eat the assignment operator

                    DefinitionBody body = parse_definition_body();

                    create_identifier(name, body, location);
                    log("parsed let statement", LogLevel::debug_extra);
                    return ast_store_->allocate_definition(RT_Definition{UserError{}, 
                        location});
                }

                get_token();
                fail("unexpected " + current_token().get_string() + ", in let-statement");
                reset_to_top_level();
                return ast_store_->allocate_definition(RT_Definition{UserError{}, 
                        location});
            }

        case TokenType::operator_t:
            return broken_definition_helper(
                "operator overloading not yet implemented, ignoring", location);

        default:
            return broken_definition_helper(
                "unexpected token: " + current_token().get_string() + " in let definition", location);
    }
}

Definition* ParserLayer1::parse_operator_definition() {
    auto location = current_token().location;

    switch (get_token().token_type) {
        case TokenType::operator_t: {
            std::string op_string = current_token().string_value();

            if (compilation_state_->builtins_->identifier_exists(op_string)) 
                return broken_definition_helper(
                    "attempting to redefine built-in operator: " + op_string, location);

            if (compilation_state_->globals_.identifier_exists(op_string))
                return broken_definition_helper(
                    "attempting to redefine user-defined operator: " + op_string, location);

            get_token();

            if (!is_assignment_operator(current_token()))
                return broken_definition_helper(
                    "unexpected token: " + current_token().get_string() + 
                    " in operator statement, expected \"=\"", location);

            get_token();

            if (current_token().token_type != TokenType::reserved_word || 
                (current_token().string_value() != "unary" && 
                current_token().string_value() != "binary"))
                    return broken_definition_helper(
                        "unexpected token: " + current_token().get_string() + 
                        " in operator statement, expected \"unary|binary\"", location);

            unsigned int arity = current_token().string_value() == "binary" ? 2 : 1;

            get_token();

            // UNARY OPERATOR
            if (arity == 1) {
                if (current_token().token_type != TokenType::reserved_word || 
                        (current_token().string_value() != "prefix" && 
                        current_token().string_value() != "postfix"))
                    ("unexpected token: " + current_token().get_string() + 
                        " in unary operator statement, expected \"prefix|postfix\"");

                assert(current_token().get_string() == "prefix" && "postfix operators not implemented");
                // UnaryFixity fixity = current_token().get_string() == "prefix" ? UnaryFixity::prefix : UnaryFixity::postfix;
                Operator::Fixity fixity = Operator::Fixity::unary_prefix;

                get_token(); // eat the fixity specifier

                DefinitionBody body;
                if (is_block_starter(current_token())) {
                    body = parse_block_statement();
                } else {
                    body = parse_expression();
                }


                auto definition = ast_store_->allocate_definition(
                    RT_Operator{op_string, body, {fixity}, location});
                compilation_state_->globals_.create_identifier(definition);
                log("parsed let statement", LogLevel::debug_extra);
                return definition;
            }

            // BINARY OPERATOR
            if (current_token().token_type != TokenType::number)
                return broken_definition_helper("unexpected token: " + current_token().get_string() + 
                    " in unary operator statement, expected precedence specifier(positive integer)", location);

            unsigned int precedence = std::stoi(current_token().string_value());
            if (precedence >= Operator::MAX_PRECEDENCE)
                return broken_definition_helper("max operator precedence is " + 
                    std::to_string(Operator::MAX_PRECEDENCE), location);

            get_token(); // eat the precedence specifier

            DefinitionBody body;
            if (is_block_starter(current_token())) {
                body = parse_block_statement();
            } else {
                body = parse_expression();
            }

            auto definition = ast_store_->allocate_definition(
                RT_Operator{op_string, body, {Operator::Fixity::binary, precedence}, 
                        location});
            compilation_state_->globals_.create_identifier(definition);
            log("parsed let statement", LogLevel::debug_extra);
            return definition;

        }
        default:
            return broken_definition_helper("unexpected token: " + current_token().get_string() + 
                " in operator statement", location);     
    }
}

Statement* ParserLayer1::parse_assignment_statement() {
    auto location = current_token().location;

    assert(current_token().token_type == TokenType::identifier 
        && "parse_assignment_statement called with current_token that was not an identifier");

    std::string name = current_token().string_value();

    get_token();
    // TODO: change '=' to it's own TokenType
    // !!! in any case assignment operators should live in their own category
    assert(is_assignment_operator(current_token())
        && "assignment statement second token not an assignment operator");
   
    get_token(); // eat '='

    Statement* inner_statement = parse_statement();
    Statement* statement = create_statement(StatementType::assignment, location);
    statement->value = Assignment{name, inner_statement};

    // expect parse_statement to eat the semicolon etc.

    log("finished parsing assignment statement from " + statement->location.to_string(), 
        LogLevel::debug_extra);
    return statement;
}

Statement* ParserLayer1::parse_block_statement() {
    auto location = current_token().location;

    assert(is_block_starter(current_token())
        && "parse_block_statement called with current token that is not a block starter");

    unsigned int indent_at_start = indent_level_;
    unsigned int curly_brace_at_start = curly_brace_level_;

    log("start parsing block statement", LogLevel::debug_extra);
    
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
            fail("Something went wrong: " + current_token().get_string() + " is not a block starter");
            assert(false && "unhandled block started in parse_block_statement");
            return ast_store_->allocate_statement(
                {StatementType::broken, current_token().location});
    }

    get_token(); // eat start token

    Statement* statement = create_statement(StatementType::block, location);
    // fetch the substatements vector
    std::vector<Statement*>* substatements = &std::get<Block>(statement->value);

    while (current_token().token_type != ending_token   ||
            indent_level_ > indent_at_start             || // allow nested blocks
            curly_brace_level_ > curly_brace_at_start      // allow nested blocks
    ) {
        if (current_token().token_type == TokenType::eof) {
            fail("Unexpected eof while parsing block statement");
            return statement;
        }

        Statement* substatement = parse_statement();
        // discard empty statements
        if (substatement->statement_type != StatementType::empty)
            substatements->push_back(substatement);
    }

    get_token(); // eat block closer
    
    if (current_token().token_type == TokenType::semicolon)
        get_token(); // eat possible trailing semicolon

    log("finished parsing block statement from " + statement->location.to_string(), 
        LogLevel::debug_extra);

    if (substatements->size() == 1) {
        // attempt to simplify
        if (!simplify_single_statement_block(statement)) {
            statement->statement_type = StatementType::illegal;
            statement->value = Undefined{};
            return statement;
        }
    }

    // simplify empty block
    if (substatements->empty()) {
        statement->statement_type = StatementType::empty;
        statement->value = Undefined{};
        return statement;
    }

    return statement;
}

bool ParserLayer1::simplify_single_statement_block(Statement* outer) {
    log("simplifying single statement block", LogLevel::debug_extra, outer->location);

    assert(outer->statement_type == StatementType::block && 
        "ParserLayer1::collapse_single_statement_block called with a statement that's not a block");

    auto block = std::get<Block>(outer->value);

    assert(block.size() == 1 && 
        "ParserLayer1::collapse_single_statement_block called with a block of length other than 1");

    auto inner = block.back();

    if (inner->is_illegal_as_single_statement_block()) {
        fail("A block cannot be a single " + inner->log_message_string(), inner->location);
        return false;
    }

    *outer = *inner;
    ast_store_->delete_statement(inner);
    return true;
}

Statement* ParserLayer1::parse_return_statement() {
    auto location = current_token().location;

    assert(current_token().token_type == TokenType::reserved_word && current_token().string_value() == "return" 
        && "parse_return_statement called with current token other than \"return\"");

    get_token(); //eat return
    Expression* expression = parse_expression();
    Statement* statement = create_statement(StatementType::return_, location);
    statement->value = expression;

    assert(is_statement_separator(current_token()) 
        && "return statement didn't end in statement separator");
    get_token(); //eat statement separator

    log("parsed return statement", LogLevel::debug_extra);
    return statement;
}

// ----------- EXPRESSIONS ----------

Expression* ParserLayer1::parse_expression() {
    auto location = current_token().location;

    switch (current_token().token_type) {
        case TokenType::eof: {
            Expression* expression = Expression::valueless(*ast_store_,
                ExpressionType::syntax_error, location);
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

        case TokenType::lambda:
            return parse_lambda_expression();
        case TokenType::question_mark:
            return parse_ternary_expression();

        case TokenType::type_identifier: 
        case TokenType::arrow_operator:
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
            Expression* expression = parse_expression();
            
            if (current_token().token_type != TokenType::indent_block_end) {
                fail("Mismatched indents!");
                return expression;
            }

            get_token(); // eat the ending indent
            assert(indent_level_ <= starting_indent_level && "Mismatched indents");
            return expression;
        }

        case TokenType::reserved_word: {
            if (current_token().string_value() != "let") {
                fail("unknown " + current_token().get_string());
                Expression* expression = Expression::valueless(*ast_store_, 
                    ExpressionType::syntax_error, location);
                expression->value = "unknown " + current_token().get_string();
                
                get_token();
                return expression;
            }
            fail("Scoped let not yet implemented");
            Expression* expression = Expression::valueless(*ast_store_, 
                ExpressionType::syntax_error, location);
            expression->value = "Scoped let not yet implemented";
            
            get_token();
            return expression;
        }
            
        default:
            fail("unexpected " + current_token().get_string() + ", at the start of an expression");
            Expression* expression = Expression::valueless(*ast_store_, ExpressionType::syntax_error, location);
            expression->value = "unexpected " + current_token().get_string() + ", at the start of an expression";

            get_token();
            return expression;
    }
}

// expects to be called with the first term as current
// tied expression = no whitespace
Expression* ParserLayer1::parse_termed_expression(bool in_tied_expression) {
    Expression* expression = Expression::termed(*ast_store_, {}, current_token().location);

    log(
        in_tied_expression ? "start parsing tied expression" : "start parsing termed expression", 
        LogLevel::debug_extra);

    expression->terms().push_back(parse_term(in_tied_expression));
    
    if (!expression->terms().back()->is_allowed_in_type_declaration())
        expression->mark_not_type_declaration();

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
                
            case TokenType::colon: {
                assert(!in_tied_expression && "colon shouldn't be tieable");

                // colon has to add parenthesis around left side as well
                if (expression->terms().size() > 1) {
                    Expression* lhs = Expression::termed(*ast_store_, {}, expression->location);
                    *lhs = *expression;
                    expression->terms() = {lhs};
                    std::get<TermedExpressionValue>(expression->value).is_type_declaration = 
                        DeferredBool::maybe_;
                }

                // eat the ":" and any following ones as they wouldn't do anything
                while (current_token().token_type == TokenType::colon) get_token(); 
                
                expression->terms().push_back(parse_termed_expression(false));

                if (!expression->terms().back()->is_allowed_in_type_declaration())
                    expression->mark_not_type_declaration();
            }

            case TokenType::lambda:
            case TokenType::parenthesis_open:
            case TokenType::bracket_open:
            case TokenType::curly_brace_open:
                expression->terms().push_back(parse_term(in_tied_expression));
                if (!expression->terms().back()->is_allowed_in_type_declaration())
                    expression->mark_not_type_declaration();
                break;

            case TokenType::string_literal:
            case TokenType::number:
            case TokenType::identifier:
            case TokenType::operator_t:
            case TokenType::indent_block_start:
            case TokenType::type_identifier:
            case TokenType::arrow_operator:
                expression->terms().push_back(parse_term(in_tied_expression));
                if (!expression->terms().back()->is_allowed_in_type_declaration())
                    expression->mark_not_type_declaration();
                break;

            default:
                fail("unexpected: " + current_token().get_string() + ", in termed expression");
                Expression* term = Expression::valueless(*ast_store_, 
                    ExpressionType::not_implemented, current_token().location);
                term->value = current_token().get_string();
                expression->terms().push_back(term);
                
                get_token();
        }
    }

    // unwrap redundant parentheses
    if (expression->terms().size() == 1) {
        auto term = expression->terms().at(0);
        ast_store_->delete_expression(expression);

        log("removed \"parentheses\" from " + expression->location.to_string(), 
            LogLevel::debug_extra);
        return term;
    }

    // handle possible binding type declaration
    if (expression->terms().size() == 2) {
        Expression* lhs = expression->terms().at(0);
        Expression* rhs = expression->terms().at(1);

        if ( rhs->is_type_declaration() != DeferredBool::true_ &&
             lhs->is_type_declaration() != DeferredBool::false_
        ) compilation_state_->possible_binding_type_declarations_.push_back(lhs);
    }

    compilation_state_->unparsed_termed_expressions_.push_back(expression);
    
    log("finished parsing termed expression from " + expression->location.to_string(), 
        LogLevel::debug_extra);
    return expression;
}

// Expects not to be called if the current token is not parseable into a term
Expression* ParserLayer1::parse_term(bool is_tied) {
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

        case TokenType::colon:
            fail("unhandled token type: " + current_token().get_string() + 
                ", reached ParserLayer1::parse_term");
            assert(false && "colons should be handled by parse_termed expression");
            return Expression::valueless(*ast_store_, ExpressionType::syntax_error, 
                current_token().location);

        case TokenType::parenthesis_open: 
            return parse_parenthesized_expression();

        case TokenType::bracket_open:
        case TokenType::curly_brace_open:
            return parse_mapping_literal();
        
        case TokenType::indent_block_start:
            return parse_expression();
            
        case TokenType::operator_t: {
            // handle minus sign as a special case
            if (current_token().string_value() == "-") {
                auto location = current_token().location;
                get_token();
                return Expression::minus_sign(*ast_store_, location);
            }

            Expression* expression = Expression::operator_identifier(*compilation_state_, 
                current_token().string_value(), current_token().location);
            get_token();
            return expression;
        }

        case TokenType::type_identifier:
            return handle_type_identifier();

        case TokenType::arrow_operator:
            return Expression::type_operator_identifier(*compilation_state_, 
                current_token().string_value(), current_token().location);

        case TokenType::question_mark:
            return parse_ternary_expression();

        case TokenType::lambda:
            return parse_lambda_expression();

        default:
            fail("unhandled token type: " + current_token().get_string() + 
                ", reached ParserLayer1::parse_term");
            assert(false && "unhandled token type in parse_term");
            return Expression::valueless(*ast_store_, ExpressionType::syntax_error, 
                current_token().location);
    }
}

Expression* ParserLayer1::parse_access_expression() {
    // TODO: eat until the closing character
    Expression* expression = Expression::valueless(*ast_store_, 
        ExpressionType::not_implemented, current_token().location);
    get_token();
    return expression;
}

Expression* ParserLayer1::parse_ternary_expression() {
    assert(false && "not implemented");
}

Expression* ParserLayer1::parse_lambda_expression() {
    auto location = current_token().location;
    get_token(); // eat the '\'

    Expression* btd = parse_binding_type_declaration();

    if (current_token().token_type != TokenType::arrow_operator)
        return fail_expression("Undexpected " + current_token().get_string() + 
            " in lambda expression, expected a \"->\" or \"=>\" ");
    get_token();

    DefinitionBody body = parse_definition_body();

    return Expression::lambda(*ast_store_, btd, body, location);
}

Expression* ParserLayer1::parse_binding_type_declaration() {
    assert(false && "not implemented");

    // just go to termed expression
}

// expects to be called with the opening parenthese as the current_token_
Expression* ParserLayer1::parse_parenthesized_expression() {
    get_token(); // eat '('

    if (current_token().token_type == TokenType::parenthesis_close) {
        fail("Empty parentheses in an expression");
        Expression* expression = Expression::valueless(*ast_store_, 
            ExpressionType::syntax_error, current_token().location);
        get_token();
        return expression;
    }

    Expression* expression = parse_expression();
    if (current_token().token_type != TokenType::parenthesis_close) {
        fail("Mismatched parentheses");
        return expression;
    }

    get_token(); // eat ')'
    return expression;
}

Expression* ParserLayer1::parse_mapping_literal() {
    Expression* expression = Expression::valueless(*ast_store_, ExpressionType::not_implemented, 
        current_token().location);
    get_token();
    // eat until the closing character

    fail("parsed mapping literal (not implemented)");

    return expression;
}

Expression* ParserLayer1::handle_string_literal() {
    Expression* expression = Expression::string_literal(*ast_store_, current_token().string_value(), 
        current_token().location);

    get_token();
    
    log("parsed string literal", LogLevel::debug_extra);
    return expression;
}

Expression* ParserLayer1::handle_numeric_literal() {
    Expression* expression = Expression::numeric_literal(*ast_store_, current_token().string_value(), 
        current_token().location);
    
    get_token();
        
    log("parsed numeric literal", LogLevel::debug_extra);
    return expression;
}

Expression* ParserLayer1::handle_identifier() {
    Expression* expression = Expression::identifier(*compilation_state_, 
        current_token().string_value(), current_token().location);
   
    get_token();
    
    log("parsed unresolved identifier", LogLevel::debug_extra);
    return expression;
}

Expression* ParserLayer1::handle_type_identifier() {
    Expression* expression = Expression::type_identifier(*compilation_state_, 
        current_token().string_value(), current_token().location);
   
    get_token();
    
    log("parsed unresolved type identifier", LogLevel::debug_extra);
    return expression;
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
//             assert(false && "Parser::parse_call_expression called with an expression that's not definition");
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
    log("resetting to global scope", LogLevel::debug);

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

} // namespace Maps
