#ifndef __PARSER_LAYER_1_HH
#define __PARSER_LAYER_1_HH

#include <array>
#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "mapsc/source.hh"
#include "mapsc/loglevel_defs.hh"

#include "mapsc/ast/callable.hh"
#include "mapsc/parser/token.hh"
#include "mapsc/parser/lexer.hh"

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
    std::optional<Callable*> eval_parse(std::istream& source_is);

// protected for unit tests
protected:
    void run_parse(std::istream& source_is);
    void prime_tokens();

    // gets the next token from the lexer and stores it in current_token_
    Token get_token();
    Token current_token() const;
    Token peek() const;

    void update_brace_levels(Token token);
    // declare the program invalid. Parsing may still continue, but no final output should be produced
    void declare_invalid();

    void fail(const std::string& message);
    void fail(const std::string& message, SourceLocation location);
    
    void log_info(const std::string& message, 
        MessageType message_type = MessageType::general_info) const;
    
    void log_info(const std::string& message, MessageType message_type, 
        SourceLocation location) const;

    // ---- IDENTIFIERS -----
    bool identifier_exists(const std::string& name) const;

    void create_identifier(const std::string& name, CallableBody body, SourceLocation location);
    void create_identifier(const std::string& name, SourceLocation location);
    std::optional<Callable*> lookup_identifier(const std::string& name);

    // mark down the location for logging purposes
    void expression_start();
    void expression_end();

    void statement_start();

    // creates an expression using ast_, marking the location as the current_expression_start_
    Statement* create_statement(StatementType statement_type);

    void handle_pragma();

    Statement* broken_statement_helper(const std::string& message);

    void parse_top_level_statement();
    Statement* parse_non_global_statement();
    Statement* parse_statement();

    Statement* parse_expression_statement();
    Statement* parse_let_statement();
    Statement* parse_operator_definition();
    Statement* parse_assignment_statement();
    Statement* parse_return_statement();
    Statement* parse_block_statement();

    // attempts to collapse a single statement block
    bool simplify_single_statement_block(Statement* outer);

    Expression* parse_expression();

    Expression* parse_termed_expression(bool is_tied = false);
    Expression* parse_term(bool is_tied = false);
    // AST::Expression* parse_call_expression(const std::string& callee);
    // Takes an expression for the callee, and optionally a signature if it's known
    // AST::Expression* parse_call_expression(AST::Expression* callee, const std::vector<AST::Type*>& signature = {});
    // std::vector<AST::Expression*> parse_argument_list();

    Expression* parse_parenthesized_expression();
    Expression* parse_mapping_literal();
    Expression* parse_access_expression();

    // ----- TERMINALS -----
    Expression* handle_string_literal();
    Expression* handle_numeric_literal();
    Expression* handle_identifier();
    Expression* handle_type_identifier();
    Expression* handle_type_constructor_identifier();

    void reset_to_top_level();
    
    std::unique_ptr<Lexer> lexer_;
    CompilationState* const compilation_state_;
    AST_Store* const ast_store_;
    PragmaStore* const pragmas_;
    
    int which_buf_slot_ = 0;
    std::array<Token, 2> token_buf_ = { Token::dummy_token, Token::dummy_token };

    std::vector<SourceLocation> current_expression_start_;
    std::vector<SourceLocation> current_statement_start_;

    bool force_top_level_eval_ = false;

    // these are automatically incremented and decremented by the get_token()
    unsigned int indent_level_ = 0;
    unsigned int parenthese_level_ = 0;
    unsigned int curly_brace_level_ = 0;
    unsigned int angle_bracket_level_ = 0;

    bool finished_ = false;

    // #ifdef DOCTEST_LIBRARY_INCLUDED
    
    // #include "layer1_tests.hh"

    // #endif
};

} // namespace Maps

#endif