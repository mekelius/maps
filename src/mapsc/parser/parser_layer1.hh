#ifndef __PARSER_LAYER_1_HH
#define __PARSER_LAYER_1_HH

#include <array>
#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "mapsc/source.hh"

#include "mapsc/logging.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/parser/token.hh"
#include "mapsc/parser/lexer.hh"
#include "mapsc/ast/chunk.hh"


namespace Maps {

class AST_Store;
class CompilationState;
class PragmaStore;
enum class StatementType;
struct Expression;
struct Statement;

class ParserLayer1 {
public:
    ParserLayer1(CompilationState* const state);

    // if fails, sets ast->is_valid to false
    bool run(std::istream& source_is);
    std::optional<Definition*> eval_parse(std::istream& source_is);

// protected for unit tests
protected:
    void run_parse(std::istream& source_is);
    void prime_tokens();

    // gets the next token from the lexer and stores it in current_token_
    Token get_token();
    const Token& current_token() const;
    const Token& peek() const;

    void update_brace_levels(Token token);
    // declare the program invalid. Parsing may still continue, but no final output should be produced

    void reset_to_top_level();
    
    void fail(const std::string& message, SourceLocation location, bool compiler_error = false);
    Expression* fail_expression(const std::string& message, SourceLocation location, 
        bool compiler_error = false);
    Definition* fail_definition(const std::string& message, SourceLocation location, 
        bool compiler_error = false);
    Statement* fail_statement(const std::string& message, SourceLocation location, 
        bool compiler_error = false);
    
    void log(const std::string& message, LogLevel loglevel) const;
    void log(const std::string& message, LogLevel loglevel, SourceLocation location) const;

    // ---- IDENTIFIERS -----
    bool identifier_exists(const std::string& name) const;

    void create_identifier(const std::string& name, DefinitionBody body, SourceLocation location);
    void create_identifier(const std::string& name, SourceLocation location);
    std::optional<Definition*> lookup_identifier(const std::string& name);

    // attempts to collapse a single statement block
    bool simplify_single_statement_block(Statement* outer);

    Definition* create_definition(DefinitionBody body, SourceLocation location);
    // creates an expression using ast_, marking the location as the expression_location_stack_
    Statement* create_statement(StatementType statement_type, SourceLocation location);

    void handle_pragma();

    Chunk parse_top_level_chunk();
    
    Definition* parse_let_definition();
    Definition* parse_operator_definition();
    DefinitionBody parse_definition_body();
    
    Statement* parse_statement();

    Statement* parse_expression_statement();
    Statement* parse_assignment_statement();
    Statement* parse_return_statement();
    Statement* parse_block_statement();

    Expression* parse_expression();

    Expression* parse_termed_expression(bool is_tied = false);
    Expression* parse_term(bool is_tied = false);

    Expression* parse_parenthesized_expression();
    Expression* parse_mapping_literal();
    Expression* parse_access_expression();
    Expression* parse_ternary_expression();
    Expression* parse_lambda_expression();

    Expression* parse_binding_type_declaration();

    // ----- TERMINALS -----
    Expression* handle_string_literal();
    Expression* handle_numeric_literal();
    Expression* handle_identifier();
    Expression* handle_type_identifier();
    Expression* handle_type_constructor_identifier();
    
    std::unique_ptr<Lexer> lexer_;
    CompilationState* const compilation_state_;
    AST_Store* const ast_store_;
    PragmaStore* const pragmas_;
    
    int which_buf_slot_ = 0;
    std::array<Token, 2> token_buf_ = { Token::dummy_token, Token::dummy_token };

    bool force_top_level_eval_ = false;

    // these are automatically incremented and decremented by the get_token()
    unsigned int indent_level_ = 0;
    unsigned int parenthese_level_ = 0;
    unsigned int curly_brace_level_ = 0;
    unsigned int angle_bracket_level_ = 0;
};

} // namespace Maps

#endif