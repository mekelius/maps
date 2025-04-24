#ifndef __PARSER_LAYER_1_HH
#define __PARSER_LAYER_1_HH

#include <istream>
#include <memory>
#include <array>
#include <ostream>
#include <vector>
#include <unordered_map>

#include "../lang/pragma.hh"
#include "../lang/ast.hh"
#include "../logging.hh"

#include "lexer.hh"

// First attempt at a parser. Parses tokens directly into the llvm context
class ParserLayer1 {
  public:
    ParserLayer1(StreamingLexer* lexer, Pragma::Pragmas* pragmas, bool in_repl = false);
    
    std::unique_ptr<Maps::AST> run();
  private:
    // gets the next token from the lexer and stores it in current_token_
    Token get_token();
    Token current_token() const;
    Token peek() const;

    void update_brace_levels(Token token);
    // declare the program invalid. Parsing may still continue, but no final output should be produced
    void declare_invalid();

    // TODO: refactor these
    void log_error(const std::string& message) const;
    void log_error(SourceLocation location, const std::string& message) const;
    
    void log_info(const std::string& message, 
        Logging::MessageType message_type = Logging::MessageType::general_info) const;
    
    void log_info(SourceLocation location, const std::string& message, 
        Logging::MessageType message_type = Logging::MessageType::general_info) const;

    // ---- IDENTIFIERS -----
    bool identifier_exists(const std::string& name) const;

    void create_identifier(const std::string& name, Maps::CallableBody body, SourceLocation location);
    void create_identifier(const std::string& name, SourceLocation location);
    std::optional<Maps::Callable*> lookup_identifier(const std::string& name);

    // mark down the location for logging purposes
    void expression_start();
    void expression_end();

    void statement_start();

    // creates an expression using ast_, marking the location as the current_expression_start_
    Maps::Statement* create_statement(Maps::StatementType statement_type);

    void handle_pragma();

    Maps::Statement* broken_statement_helper(const std::string& message);

    void parse_top_level_statement();
    Maps::Statement* parse_non_global_statement();
    Maps::Statement* parse_statement();

    Maps::Statement* parse_expression_statement();
    Maps::Statement* parse_let_statement();
    Maps::Statement* parse_operator_statement();
    Maps::Statement* parse_assignment_statement();
    Maps::Statement* parse_return_statement();
    Maps::Statement* parse_block_statement();

    Maps::Expression* parse_expression();

    Maps::Expression* parse_termed_expression(bool is_tied = false);
    Maps::Expression* parse_term(bool is_tied = false);
    // the token has to be an identifier. The caller must also be certain that this is a call expression
    // i.e. call this only when the current token is an identifier as well
    // AST::Expression* parse_call_expression(const std::string& callee);
    // Takes an expression for the callee, and optionally a signature if it's known
    // DO NOT pass nullptr
    // AST::Expression* parse_call_expression(AST::Expression* callee, const std::vector<AST::Type*>& signature = {});
    // std::vector<AST::Expression*> parse_argument_list();

    Maps::Expression* parse_parenthesized_expression();
    Maps::Expression* parse_mapping_literal();
    Maps::Expression* parse_access_expression();

    // ----- TERMINALS -----
    Maps::Expression* handle_string_literal();
    Maps::Expression* handle_numeric_literal();
    Maps::Expression* handle_identifier();

    void reset_to_top_level();
    
    StreamingLexer* lexer_;
    std::unique_ptr<Maps::AST> ast_;
    Pragma::Pragmas* pragmas_;
    
    int which_buf_slot_ = 0;
    std::array<Token, 2> token_buf_ = { Token::dummy_token, Token::dummy_token };

    std::vector<SourceLocation> current_expression_start_;
    std::vector<SourceLocation> current_statement_start_;

    bool in_repl_ = false;

    // these are automatically incremented and decremented by the get_token()
    unsigned int indent_level_ = 0;
    unsigned int parenthese_level_ = 0;
    unsigned int curly_brace_level_ = 0;
    unsigned int angle_bracket_level_ = 0;

    bool finished_ = false;
};

#endif