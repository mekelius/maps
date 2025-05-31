#include "implementation.hh"

#include <cassert>
#include <initializer_list>
#include <sstream>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/compilation_state.hh"

#include "mapsc/ast/misc_expression.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/ast_store.hh"

#include "mapsc/parser/token.hh"
#include "mapsc/procedures/simplify.hh"

using std::optional, std::nullopt, std::make_unique, std::to_string;


namespace Maps {

using Log = LogInContext<LogContext::layer1>;

// ----- PUBLIC METHODS -----

ParserLayer1::ParserLayer1(CompilationState* const state, Scope* scope)
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

    if ((*result_.top_level_definition))
        simplify(*(*result_.top_level_definition));

    if (!result_.top_level_definition) 
        result_.top_level_definition = nullopt;

    return result_;
}

// ----- PRIVATE METHODS -----

void ParserLayer1::run_parse(std::istream& source_is) {
    lexer_ = make_unique<Lexer>(&source_is);

    prime_tokens();

    auto location = current_token().location;
    Statement* root_statement = create_block(*ast_store_, {}, location);
    result_.top_level_definition = *ast_store_->allocate_definition(DefinitionHeader{
        "root", &Hole, parse_scope_, true, location}, root_statement)->body_;

    context_stack_.push_back(parse_scope_);

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
    // manage the parentheses and indents etc.
    update_brace_levels(current_token());

    token_buf_[which_buf_slot_ % 2] = lexer_->get_token();
    which_buf_slot_++;

    return current_token();
}

const Token& ParserLayer1::current_token() const {
    return token_buf_[which_buf_slot_ % 2];
}

const Token& ParserLayer1::peek() const {
    return token_buf_[(which_buf_slot_+1) % 2];
}

bool ParserLayer1::eof() const {
    return current_token().token_type == TokenType::eof;
}

bool ParserLayer1::has_failed() const {
    return !result_.success;
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

        default: break;
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
        create_compiler_error(*ast_store_, location) : 
        create_user_error(*ast_store_, location);
}
 
DefinitionHeader* ParserLayer1::fail_definition(const std::string& message, SourceLocation location, 
    bool compiler_error) {

    assert(false && "not updated");
    
    // fail(message, location, compiler_error);
    // return create_definition(Error{compiler_error}, true, location);
}

Statement* ParserLayer1::fail_statement(const std::string& message, SourceLocation location, 
    bool compiler_error) {
    
    fail(message, location, compiler_error);
    return compiler_error ? 
        create_compiler_error_statement(*ast_store_, location)
        : create_user_error_statement(*ast_store_, location);
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

void ParserLayer1::push_context(Scope* context) {
    context_stack_.push_back(context);
}

std::optional<Scope*> ParserLayer1::pop_context() {
    if (context_stack_.empty()) {
        Log::compiler_error("context stack shouldn't be empty", NO_SOURCE_LOCATION);
        assert(false && "context stack shouldn't be empty");
        return nullopt;
    }

    if (context_stack_.size() == 1)
        return context_stack_.back();

    auto context = context_stack_.back();
    context_stack_.pop_back();
    return context;
}

std::optional<Scope*> ParserLayer1::current_context() const {
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

optional<DefinitionHeader*> ParserLayer1::create_undefined_identifier(const std::string& name, bool is_top_level, SourceLocation location) {
    return parse_scope_->create_identifier(
        ast_store_->allocate_definition(DefinitionHeader{name, &Hole, parse_scope_, is_top_level, location})
    );
}

optional<DefinitionHeader*> ParserLayer1::create_identifier(DefinitionHeader* definition) {
    return parse_scope_->create_identifier(definition);
}

std::optional<DefinitionHeader*> ParserLayer1::lookup_identifier(const std::string& identifier) {
    return parse_scope_->get_identifier(identifier);
}

// ----- CREATION HELPERS -----

DefinitionHeader* ParserLayer1::create_definition(const std::string& name, 
    const LetDefinitionValue& body_value, bool is_top_level, SourceLocation location) {
    
    return ast_store_->allocate_definition(
        DefinitionHeader{name, &Hole, parse_scope_, is_top_level, location}, body_value);
}

DefinitionHeader* ParserLayer1::create_definition(const std::string& name, bool is_top_level, 
    SourceLocation location) {

    return ast_store_->allocate_definition(DefinitionHeader{name, &Hole, parse_scope_, is_top_level, location}); 
}

DefinitionHeader* ParserLayer1::create_definition(LetDefinitionValue body, bool is_top_level, 
    SourceLocation location) {
    
    assert(false && "not updated");
    // return ast_store_->allocate_definition(DefinitionHeader{is_top_level, location});
}

} // namespace Maps
