#ifndef __LAYER1_IMPLEMENTATION_HH
#define __LAYER1_IMPLEMENTATION_HH

#include "../layer1.hh"

#include <array>
#include <istream>
#include <memory>
#include <optional>
#include <utility>
#include <string>
#include <vector>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/definition_body.hh"
#include "mapsc/ast/function_definition.hh"
#include "mapsc/parser/token.hh"
#include "mapsc/parser/lexer.hh"
#include "mapsc/ast/chunk.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/statement.hh"

namespace Maps {

class AST_Store;
class CompilationState;
class PragmaStore;

enum class StatementType;

class ParserLayer1 {
public:
    ParserLayer1(CompilationState* const state, Scope* scope);

    Layer1Result run(std::istream& source_is);
    Layer1Result run_eval(std::istream& source_is);

// protected for unit tests
protected:
    void run_parse(std::istream& source_is);
    void prime_tokens();

    // gets the next token from the lexer and stores it in current_token_
    Token get_token();
    const Token& current_token() const;
    const Token& peek() const;
    bool eof() const;
    bool has_failed() const;

    void update_brace_levels(Token token);
    void reset_to_top_level();

    void push_context(Scope* context);
    std::optional<Scope*> pop_context();
    std::optional<Scope*> current_context() const;
    bool in_top_level_context() const { return context_stack_.size() == 1; }
    
    void fail();
    void fail(std::string_view message, const SourceLocation& location);
    Expression* fail_expression(SourceLocation location, bool compiler_error = false);
    Expression* fail_expression(std::string_view message, SourceLocation location, bool compiler_error = false);
    DefinitionHeader* fail_definition(SourceLocation location, bool compiler_error = false);
    DefinitionHeader* fail_definition(std::string_view message, SourceLocation location, bool compiler_error = false);
    Statement* fail_statement(SourceLocation location, bool compiler_error = false);
    Statement* fail_statement(std::string_view message, SourceLocation location, bool compiler_error = false);
    std::nullopt_t fail_optional();
    std::nullopt_t fail_optional(std::string_view message, const SourceLocation& location, bool compiler_error = false);
    
    void log(const std::string& message, LogLevel loglevel) const;
    void log(const std::string& message, LogLevel loglevel, SourceLocation location) const;

    // ---- IDENTIFIERS -----
    bool identifier_exists(const std::string& name) const;

    [[nodiscard]] std::optional<DefinitionHeader*> create_undefined_identifier(const std::string& name, 
        bool is_top_level, SourceLocation location);
    [[nodiscard]] std::optional<DefinitionHeader*> create_identifier(DefinitionHeader* definition);
    std::optional<DefinitionHeader*> lookup_identifier(const std::string& name);

    // attempts to collapse a single statement block
    bool simplify_single_statement_block(Statement* outer);

    DefinitionHeader* create_definition(const std::string& name, const LetDefinitionValue& definition, 
        bool is_top_level, SourceLocation location);
    DefinitionHeader* create_definition(const std::string& name, bool is_top_level, 
        SourceLocation location);
    DefinitionHeader* create_definition(LetDefinitionValue body, bool is_top_level, 
        SourceLocation location);

    // #################################### layer1/pragma.cpp #####################################

    void handle_pragma();

    // ################################## layer1/definition.cpp ###################################

    Chunk parse_top_level_chunk();
    DefinitionHeader* parse_top_level_let_definition();
    Operator* parse_operator_definition();
    LetDefinitionValue parse_definition_body();
    Statement* parse_inner_let_definition();
    
    // ################################## layer1/statement.cpp ####################################

    Statement* parse_statement();
    Statement* parse_expression_statement();
    Statement* parse_assignment_statement();
    Statement* parse_return_statement();
    Statement* parse_block_statement();

    Statement* parse_if_statement();
    Statement* parse_conditional_body();
    Statement* parse_else_branch(uint initial_indent);

    Statement* parse_while_loop();
    Statement* parse_for_loop();
    Statement* parse_guard_statement();
    Statement* parse_switch_statement();
    Statement* parse_yield_statement();

    // ################################# layer1/expression.cpp ####################################

    Expression* parse_expression();
    Expression* parse_parenthesized_expression();
    Expression* parse_mapping_literal();
    Expression* parse_access_expression();
    Expression* parse_ternary_expression();
    Expression* parse_lambda_expression();
    Expression* parse_condition_expression();

    std::optional<ParameterList> parse_lambda_parameters(Scope* lambda_scope);
    Expression* parse_binding_type_declaration();

    std::optional<const Type*> parse_parameter_type_declaration();

    // ############################## layer1/layer2_expression.cpp ################################

    Expression* parse_termed_expression(bool is_tied = false);
    Expression* parse_term(bool is_tied = false);
    Expression* close_termed_expression(Expression* expression);

    // ################################## layer1/terminal.cpp #####################################
    
    Expression* handle_string_literal();
    Expression* handle_numeric_literal();
    Expression* handle_identifier();
    Expression* handle_type_identifier();
    Expression* handle_type_constructor_identifier();
        
    // ------------------------------------ PRIVATE FIELDS ----------------------------------------

    std::unique_ptr<Lexer> lexer_;
    Layer1Result result_ = {};

    CompilationState* const compilation_state_;
    Scope* parse_scope_;
    
    AST_Store* const ast_store_;
    PragmaStore* const pragma_store_;
    
    int which_buf_slot_ = 0;
    std::array<Token, 2> token_buf_ = { Token::dummy_token, Token::dummy_token };

    bool force_top_level_eval_ = false;

    // these are automatically incremented and decremented by the get_token()
    unsigned int indent_level_ = 0;
    unsigned int parenthese_level_ = 0;
    unsigned int curly_brace_level_ = 0;
    unsigned int angle_bracket_level_ = 0;

    std::vector<Scope*> context_stack_{};
};

} // namespace Maps

#endif