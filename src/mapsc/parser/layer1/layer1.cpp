#include "implementation.hh"

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

Layer1Result ParserLayer1::run(std::istream& source_is) {    
    run_parse(source_is);
    return result_;
}

Layer1Result ParserLayer1::run_eval(std::istream& source_is) {    
    force_top_level_eval_ = true;
    run_parse(source_is);
    force_top_level_eval_ = false;

    if (result_.top_level_definition)
        simplify(**result_.top_level_definition);

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
        "root", root_statement, true, {0,0}});

    context_stack_.push_back(*result_.top_level_definition);

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
    return create_definition(Error{compiler_error}, true, location);
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

    context_stack_ = {};

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

void ParserLayer1::push_context(RT_Definition* context) {
    context_stack_.push_back(context);
}

std::optional<RT_Definition*> ParserLayer1::pop_context() {
    if (context_stack_.empty())
        return nullopt;

    auto context = context_stack_.back();
    context_stack_.pop_back();
    return context;
}

std::optional<RT_Definition*> ParserLayer1::current_context() const {
    if (context_stack_.empty())
        return nullopt;

    return context_stack_.back();
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

// ----- IDENTIFIERS -----

bool ParserLayer1::identifier_exists(const std::string& identifier) const {
    if (compilation_state_->builtins_->identifier_exists(identifier))
        return true;

    if (parse_scope_->identifier_exists(identifier))
        return true;

    return false;
}

optional<RT_Definition*> ParserLayer1::create_undefined_identifier(const std::string& name, bool is_top_level, SourceLocation location) {
    return parse_scope_->create_identifier(
        ast_store_->allocate_definition(RT_Definition{name, Undefined{}, is_top_level, location})
    );
}

optional<RT_Definition*> ParserLayer1::create_identifier(RT_Definition* definition) {
    return parse_scope_->create_identifier(definition);
}

std::optional<Definition*> ParserLayer1::lookup_identifier(const std::string& identifier) {
    return parse_scope_->get_identifier(identifier);
}

// ----- CREATION HELPERS -----

RT_Definition* ParserLayer1::create_definition(const std::string& name, DefinitionBody body, 
    bool is_top_level, SourceLocation location) {
    
    return ast_store_->allocate_definition(RT_Definition{name, body, is_top_level, location});
}

RT_Definition* ParserLayer1::create_definition(const std::string& name, bool is_top_level, 
    SourceLocation location) {

    return ast_store_->allocate_definition(RT_Definition{name, Undefined{}, is_top_level, location}); 
}

RT_Definition* ParserLayer1::create_definition(DefinitionBody body, bool is_top_level, 
    SourceLocation location) {
    
    return ast_store_->allocate_definition(RT_Definition{body, is_top_level, location});
}

Statement* ParserLayer1::create_statement(StatementType statement_type, SourceLocation location) {
    Statement* statement = ast_store_->allocate_statement({statement_type, location});
    return statement;
}

} // namespace Maps
