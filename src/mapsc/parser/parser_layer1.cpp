/**
 * Recursive descent parser for maps programming language
 * 
 * The convention here (unlike in the lexer) is that every production rule must move the buffer
 * beyond the tokens it consumed.
 * 
 * Same if a token is rejected. 
 */
#include "parser_layer1.hh"

#include <algorithm>
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

ParserLayer1::ParserLayer1(CompilationState* const state, RT_Scope* scope)
:compilation_state_(state), 
 parse_scope_(scope),
 ast_store_(state->ast_store_.get()),
 pragma_store_(&state->pragmas_) {}

ParserLayer1::Result ParserLayer1::run(std::istream& source_is) {    
    run_parse(source_is);
    return result_;
}

ParserLayer1::Result ParserLayer1::run_eval(std::istream& source_is) {    
    force_top_level_eval_ = true;
    run_parse(source_is);
    force_top_level_eval_ = false;

    if (result_.top_level_definition)
        attempt_simplify(**result_.top_level_definition);

    if (!result_.top_level_definition                   || 
        (*result_.top_level_definition)->is_undefined() ||
        (*result_.top_level_definition)->is_empty()
    ) result_.top_level_definition = nullopt;

    return result_;
}

// ----- PRIVATE METHODS -----

void ParserLayer1::run_parse(std::istream& source_is) {
    lexer_ = make_unique<Lexer>(&source_is);

    prime_tokens();

    Statement* root_statement = ast_store_->allocate_statement({StatementType::block, {0,0}});
    result_.top_level_definition = ast_store_->allocate_definition(RT_Definition{
        "root", root_statement, {0,0}});

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

// ----- OUTPUT HELPERS -----

void ParserLayer1::fail(const std::string& message, SourceLocation location, 
    bool compiler_error) {
    
    if (compiler_error) {
        Log::compiler_error(message, location);
    } else {
        Log::error(message, location);
    }
    reset_to_top_level();
    result_.success = false;
}

Expression* ParserLayer1::fail_expression(const std::string& message, SourceLocation location, 
    bool compiler_error) {
    
    fail(message, location, compiler_error);
    get_token();
    return compiler_error ? 
        Expression::compiler_error(*ast_store_, location) : 
        Expression::user_error(*ast_store_, location);
}
 
Definition* ParserLayer1::fail_definition(const std::string& message, SourceLocation location, 
    bool compiler_error) {
    
    fail(message, location, compiler_error);
    return create_definition(Error{compiler_error}, location);
}

Statement* ParserLayer1::fail_statement(const std::string& message, SourceLocation location, 
    bool compiler_error) {
    
    fail(message, location, compiler_error);
    return create_statement(compiler_error ? 
        StatementType::compiler_error : StatementType::user_error, 
        location);
}

std::nullopt_t ParserLayer1::fail_optional(const std::string& message, SourceLocation location, 
    bool compiler_error) {

    fail(message, location, compiler_error);
    return nullopt;
}

void ParserLayer1::log(const std::string& message, LogLevel loglevel) const {
    Log::on_level(message, loglevel, current_token().location);
}

void ParserLayer1::log(const std::string& message, LogLevel loglevel, 
    SourceLocation location) const {
    
    Log::on_level(message, loglevel, location);
}

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

// ----- IDENTIFIERS -----

bool ParserLayer1::identifier_exists(const std::string& identifier) const {
    if (compilation_state_->builtins_->identifier_exists(identifier))
        return true;

    if (parse_scope_->identifier_exists(identifier))
        return true;

    return false;
}

void ParserLayer1::create_identifier(const std::string& name, SourceLocation location) {
    create_identifier(name, Undefined{}, location);
}

void ParserLayer1::create_identifier(const std::string& name,
    DefinitionBody body, SourceLocation location) {
    log("Parsed identifier " + name, LogLevel::debug_extra);
    parse_scope_->create_identifier(
        ast_store_->allocate_definition(RT_Definition{name, body, location})
    );
}

std::optional<Definition*> ParserLayer1::lookup_identifier(const std::string& identifier) {
    return parse_scope_->get_identifier(identifier);
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
    auto location = current_token().location;

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
        fail("invalid pragma declaration", location);
        get_token();
        return;
    }

    // the rest should be the flag name
    bool succeeded = pragma_store_->set_flag(
        flag_name, value, current_token().location);
    
    if (!succeeded) 
        fail("handling pragma failed", location);

    get_token();
}

Chunk ParserLayer1::parse_top_level_chunk() {
    Statement* statement;

    switch (current_token().token_type) {
        case TokenType::pragma:
            handle_pragma();
            return parse_top_level_chunk();

        case TokenType::eof:
            return std::monostate{};

        case TokenType::reserved_word:
            if (current_token().string_value() == "let") {
                log("Scoping not yet implemented", LogLevel::warning);
                return parse_top_level_let_definition();
            }

            if (current_token().string_value() == "operator")
                return parse_operator_definition();
            
            assert(false && "Unhandled reserved word in Parser::parse_top_level_statement");
            

        default:
            statement = parse_statement();
            if (statement->statement_type != StatementType::empty && !statement->is_definition() &&
                (pragma_store_->check_flag_value("top-level evaluation", statement->location) || 
                    force_top_level_eval_)) {            
                std::get<Block>(std::get<Statement*>(
                    (*result_.top_level_definition)->body())->value)
                        .push_back(statement);
            }
            return statement;
    }
}

DefinitionBody ParserLayer1::parse_definition_body() {
    return is_block_starter(current_token()) ?
        DefinitionBody{parse_block_statement()} : DefinitionBody{parse_expression()};
}

Statement* ParserLayer1::parse_statement() {
    auto location = current_token().location;

    switch (current_token().token_type) {
        case TokenType::eof:
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
            return fail_statement("parse_statement called at indent_block_end", location, true);
        
        case TokenType::indent_error_fatal:
            return fail_statement("indent error", location);

        case TokenType::reserved_word:
            return parse_initial_reserved_word_statement();
            
        // ---- errors -----
        default:
            return fail_statement(
                "Unexpected "+ current_token().get_string() + ", expected a statement", location);
    }
}

Statement* ParserLayer1::parse_initial_reserved_word_statement() {
    auto reserved_word = current_token().string_value();

    if (reserved_word == "let") {
        return parse_inner_let_definition();

    } else if (reserved_word == "return") {
        return parse_return_statement();

    } else if (reserved_word == "if") {
        assert(false && "not implemented");
    } else if (reserved_word == "else") {
        assert(false && "not implemented");
    } else if (reserved_word == "for") {
        assert(false && "not implemented");
    } else if (reserved_word == "while") {
        assert(false && "not implemented");
    } else if (reserved_word == "do") {
        assert(false && "not implemented");
    } else if (reserved_word == "switch") {
        assert(false && "not implemented");
    } else if (reserved_word == "match") {
        assert(false && "not implemented");
    }

    return fail_statement("Unexpected reserved word " + reserved_word + " expected a statement", 
        current_token().location);
}

Statement* ParserLayer1::parse_inner_let_definition() {
    assert(false && "not implemented");
}

Statement* ParserLayer1::parse_expression_statement() {
    auto location = current_token().location;

    Expression* expression = parse_expression();
    Statement* statement = create_statement(StatementType::expression_statement, location);
    statement->value = expression;

    if(!is_block_starter(current_token()) && !is_statement_separator(current_token()))
        return fail_statement("statement didn't end in a statement separator", 
            current_token().location, true);

    if (current_token().token_type == TokenType::semicolon)
        get_token(); // eat trailing semicolon
    log("finished parsing expression statement from " + statement->location.to_string(), 
        LogLevel::debug_extra);
    return statement;
}

Definition* ParserLayer1::parse_top_level_let_definition() {
    auto location = current_token().location;
    
    switch (get_token().token_type) {
        case TokenType::identifier: {
                std::string name = current_token().string_value();
                
                // check if name already exists
                if (identifier_exists(name))
                    return fail_definition("Attempting to redefine identifier " + name, location);

                get_token(); // eat the identifier

                if (is_statement_separator(current_token())) {
                    log("parsed let definition declaring \"" + name + "\" with no definition", 
                        LogLevel::debug_extra);
                    
                    get_token(); // eat the semicolon

                    // create an unitialized identifier
                    create_identifier(name, Undefined{}, location);

                    return nullptr; //!!!
                }

                if (is_assignment_operator(current_token())) {
                    get_token(); // eat the assignment operator

                    DefinitionBody body = parse_definition_body();

                    create_identifier(name, body, location);
                    log("parsed let definition", LogLevel::debug_extra);
                    return nullptr; //!!!
                }

                get_token();
                return fail_definition(
                    "Unexpected " + current_token().get_string() + ", in let-definition", location);
            }

        case TokenType::operator_t:
            return fail_definition(
                "operator overloading not yet implemented, ignoring", location);

        default:
            return fail_definition(
                "unexpected token: " + current_token().get_string() + " in let definition", location);
    }
}

Definition* ParserLayer1::parse_operator_definition() {
    auto location = current_token().location;

    switch (get_token().token_type) {
        case TokenType::operator_t: {
            std::string op_string = current_token().string_value();

            if (identifier_exists(op_string)) 
                return fail_definition(
                    "operator: " + op_string, location);

            get_token();

            if (!is_assignment_operator(current_token()))
                return fail_definition(
                    "unexpected token: " + current_token().get_string() + 
                    " in operator statement, expected \"=\"", location);

            get_token();

            if (current_token().token_type != TokenType::reserved_word || 
                    (current_token().string_value() != "unary" && 
                        current_token().string_value() != "binary"))
                return fail_definition(
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

                assert(current_token().get_string() == "prefix" && 
                    "postfix operators not implemented");
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
                parse_scope_->create_identifier(definition);
                log("parsed let statement", LogLevel::debug_extra);
                return definition;
            }

            // BINARY OPERATOR
            if (current_token().token_type != TokenType::number)
                return fail_definition("unexpected token: " + current_token().get_string() + 
                    " in unary operator statement, expected precedence specifier(positive integer)", 
                    location);

            unsigned int precedence = std::stoi(current_token().string_value());
            if (precedence >= Operator::MAX_PRECEDENCE)
                return fail_definition("max operator precedence is " + 
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
            parse_scope_->create_identifier(definition);
            log("parsed let statement", LogLevel::debug_extra);
            return definition;

        }
        default:
            return fail_definition("unexpected token: " + current_token().get_string() + 
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
            return fail_statement("Something went wrong: " + current_token().get_string() + 
                " is not a block starter", current_token().location, true);
    }

    get_token(); // eat start token

    Statement* statement = create_statement(StatementType::block, location);
    // fetch the substatements vector
    std::vector<Statement*>* substatements = &std::get<Block>(statement->value);

    while (current_token().token_type != ending_token   ||
            indent_level_ > indent_at_start             || // allow nested blocks
            curly_brace_level_ > curly_brace_at_start      // allow nested blocks
    ) {
        if (current_token().token_type == TokenType::eof)
            return fail_statement("Unexpected eof while parsing block statement", location);

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
        if (!simplify_single_statement_block(statement))
            return fail_statement("Simplification failed", statement->location, true);
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

    assert(current_token().token_type == TokenType::reserved_word && 
        current_token().string_value() == "return" 
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
                ExpressionType::user_error, location);
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
            
            if (current_token().token_type != TokenType::indent_block_end)
                return fail_expression("Mismatched indents!", current_token().location);

            get_token(); // eat the ending indent
            assert(indent_level_ <= starting_indent_level && "Mismatched indents");
            return expression;
        }

        case TokenType::reserved_word:
            if (current_token().string_value() != "let")
                assert(false && "not implemented");
            assert(false && "not implemented");            
            
        default:
            return fail_expression(
                "unexpected " + current_token().get_string() + ", at the start of an expression",
                current_token().location);
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
                return fail_expression(
                    "unexpected: " + current_token().get_string() + ", in termed expression",
                    current_token().location);
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
        ) result_.possible_binding_type_declarations.push_back(lhs);
    }

    result_.unparsed_termed_expressions.push_back(expression);
    
    log("finished parsing termed expression from " + expression->location.to_string(), 
        LogLevel::debug_extra);
    return expression;
}

// Expects not to be called if the current token is not parseable into a term
Expression* ParserLayer1::parse_term(bool is_tied) {
    // if we see a tie, go down into a tied expression if not already in one
    if (peek().token_type == TokenType::tie && !is_tied)
        return parse_termed_expression(true);

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
                ", reached ParserLayer1::parse_term", current_token().location);
            assert(false && "colons should be handled by parse_termed expression");
            return Expression::valueless(*ast_store_, ExpressionType::user_error, 
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

            Expression* expression = Expression::operator_identifier(*ast_store_, parse_scope_,
                current_token().string_value(), current_token().location);
            result_.unresolved_identifiers.push_back(expression);

            get_token();
            return expression;
        }

        case TokenType::type_identifier:
            return handle_type_identifier();

        case TokenType::arrow_operator:
            assert(false && "not implemented");
            // return Expression::type_operator_reference(*ast_store_,
            //     current_token().string_value(), current_token().location);

        case TokenType::question_mark:
            return parse_ternary_expression();

        case TokenType::lambda:
            return parse_lambda_expression();

        default:
            assert(false && "unhandled token type in parse_term");
            return fail_expression("unhandled token type: " + current_token().get_string() + 
                ", reached ParserLayer1::parse_term", current_token().location, true);
    }
}

Expression* ParserLayer1::parse_access_expression() {
    assert(false && "not implemented");
}

Expression* ParserLayer1::parse_ternary_expression() {
    assert(false && "not implemented");
}

Expression* ParserLayer1::parse_lambda_expression() {
    auto location = current_token().location;
    get_token(); // eat the '\'

    RT_Scope* lambda_scope = ast_store_->allocate_scope(RT_Scope{/*parse_scope_*/});
    auto parameter_list = parse_lambda_parameters(lambda_scope);

    if (!parameter_list)
        return fail_expression("parsing lambda parameter list failed", location);

    if (current_token().token_type != TokenType::arrow_operator)
        return fail_expression("Undexpected " + current_token().get_string() + 
            " in lambda expression, expected a \"->\" or \"=>\" ", current_token().location);

    bool is_pure = current_token().string_value() == "->";
    get_token();

    DefinitionBody body = parse_definition_body();

    return Expression::lambda(*compilation_state_, {*parameter_list, lambda_scope, body}, is_pure, 
        location);
}

optional<ParameterList> ParserLayer1::parse_lambda_parameters(RT_Scope* lambda_scope) {
    ParameterList parameter_list{};

    while (true) {
        switch (current_token().token_type) {
            case TokenType::arrow_operator:
                return parameter_list;

            case TokenType::eof:
                return fail_optional(
                    "Unexpected eof in lambda parameter list", current_token().location);

            case TokenType::identifier: {
                auto name = current_token().string_value();
                auto location = current_token().location;

                auto parameter = RT_Definition::parameter(*ast_store_, name, &Hole, location);

                // check if the string is already bound, in which case we exit
                if (!lambda_scope->create_identifier(name, parameter))
                    return fail_optional(
                        "Duplicate parameter name " + name + " in lambda parameter list", location);

                parameter_list.push_back(parameter);
                get_token();
                continue;
            }

            case TokenType::bracket_open:
            case TokenType::type_identifier: {
                auto location = current_token().location;

                auto type = parse_parameter_type_declaration();
                
                if (!type)
                    return fail_optional(
                        "Parsing lambda parameter list failed, incorrect type declaration", location);

                if (current_token().token_type != TokenType::identifier) {
                    assert(false && "Something unexpected happened, parse_parameter_type_declaration returned without an identifier as the current token");
                    return fail_optional(("Something unexpected happened, parse_parameter_type_declaration returned without an identifier as the current token"), 
                        current_token().location, true);
                }

                auto name = current_token().string_value();
                auto parameter = RT_Definition::parameter(*ast_store_, name, *type, location);

                // check if the string is already bound, in which case we exit
                if (!lambda_scope->create_identifier(name, parameter))
                    return fail_optional(
                        "Duplicate parameter name " + name + " in lambda parameter list", location);
                get_token();
                continue;
            }

            case TokenType::underscore:
                parameter_list.push_back(RT_Definition::discarded_parameter(*ast_store_, &Hole, 
                    current_token().location));
                get_token();
                continue;

            default:
                return fail_optional(
                    "Unexpected " + current_token().get_string() + " in lambda parameter list",
                    current_token().location);
        }
    }
}

std::optional<const Type*> ParserLayer1::parse_parameter_type_declaration() {
    auto location = current_token().location;

    switch (current_token().token_type) {
        case TokenType::type_identifier: {
            switch (peek().token_type) {
                case TokenType::identifier: {
                    auto type = compilation_state_->types_->get(current_token().string_value());
                    get_token();
                    return type;
                }
                case TokenType::arrow_operator:
                    assert(false && "not implemented");
                    get_token();

                case TokenType::eof:
                    fail("Unexpected eof in parameter type declaration", current_token().location);
                    return nullopt;

                default:
                    fail("Unexpected " + current_token().get_string() + 
                        " in parameter type declaration, expected an identifier or an arrow", location);
            }
        }

        case TokenType::indent_block_start:
        case TokenType::bracket_open:
        case TokenType::parenthesis_open:
        case TokenType::colon:
            assert(false && "not implemented");

        default:
            fail("Unexpected " + current_token().get_string() + 
                " in parameter type declaration, expected a type expression", location);
            return nullopt;
    }

}

Expression* ParserLayer1::parse_binding_type_declaration() {
    assert(false && "not implemented");

    // just go to termed expression
}

// expects to be called with the opening parenthese as the current_token_
Expression* ParserLayer1::parse_parenthesized_expression() {
    get_token(); // eat '('

    if (current_token().token_type == TokenType::parenthesis_close) {
        auto location = current_token().location;
        get_token();
        return fail_expression("Empty parentheses in an expression", location);
    }

    Expression* expression = parse_expression();
    if (current_token().token_type != TokenType::parenthesis_close)
        return fail_expression("Mismatched parentheses", current_token().location);

    get_token(); // eat ')'
    return expression;
}

Expression* ParserLayer1::parse_mapping_literal() {
    assert(false && "not implemented");
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
    Expression* expression = Expression::identifier(*ast_store_, parse_scope_, 
        current_token().string_value(), current_token().location);
    result_.unresolved_identifiers.push_back(expression);

    get_token();
    
    log("parsed unresolved identifier", LogLevel::debug_extra);
    return expression;
}

Expression* ParserLayer1::handle_type_identifier() {
    Expression* expression = Expression::type_identifier(*ast_store_, 
        current_token().string_value(), current_token().location);
    result_.unresolved_type_identifiers.push_back(expression);

    get_token();
    
    log("parsed unresolved type identifier", LogLevel::debug_extra);
    return expression;
}

} // namespace Maps
