#ifndef __PARSER_HH
#define __PARSER_HH

#include <istream>
#include <memory>
#include <array>
#include <ostream>
#include <vector>
#include <unordered_map>

#include "../lang/ast.hh"
#include "../logging.hh"

#include "lexer.hh"

// First attempt at a parser. Parses tokens directly into the llvm context
class Parser {
  public:
    Parser(StreamingLexer* lexer);
    
    std::unique_ptr<AST::AST> run();
  private:
    // gets the next token from the lexer and stores it in current_token_
    Token get_token();
    Token current_token() const;
    Token peek() const;

    void update_brace_levels(Token token);
    // declare the program invalid. Parsing may still continue, but no final output should be produced
    void declare_invalid();
    std::unique_ptr<AST::AST> finalize_parsing();

    // TODO: refactor these
    void print_error(const std::string& message) const;
    void print_error(Logging::Location location, const std::string& message) const;
    void print_info(const std::string& message, Logging::MessageType message_type = Logging::MessageType::general_info) const;
    void print_info(Logging::Location location, const std::string& message, Logging::MessageType message_type = Logging::MessageType::general_info) const;

    // ---- IDENTIFIERS -----
    bool identifier_exists(const std::string& identifier) const;
    void create_identifier(const std::string& identifier, AST::CallableBody body);
    std::optional<AST::Callable*> lookup_identifier(const std::string& identifier);

    void handle_pragma();

    void parse_top_level_statement();
    AST::Statement* parse_non_global_statement();
    AST::Statement* parse_statement();

    AST::Statement* parse_expression_statement();
    AST::Statement* parse_let_statement();
    AST::Statement* parse_assignment_statement();
    AST::Statement* parse_return_statement();
    AST::Statement* parse_block_statement();

    AST::Expression* parse_expression();

    AST::Expression* parse_termed_expression();
    AST::Expression* parse_term();
    // the token has to be an identifier. The caller must also be certain that this is a call expression
    // i.e. call this only when the current token is an identifier as well
    // AST::Expression* parse_call_expression(const std::string& callee);
    // Takes an expression for the callee, and optionally a signature if it's known
    // DO NOT pass nullptr
    // AST::Expression* parse_call_expression(AST::Expression* callee, const std::vector<AST::Type*>& signature = {});
    // std::vector<AST::Expression*> parse_argument_list();

    AST::Expression* parse_parenthesized_expression();
    AST::Expression* parse_mapping_literal();
    AST::Expression* parse_access_expression();

    // ----- TERMINALS -----
    AST::Expression* handle_string_literal();
    AST::Expression* handle_numeric_literal();
    AST::Expression* handle_identifier();

    void reset_to_top_level();
    
    StreamingLexer* lexer_;
    std::unique_ptr<AST::AST> ast_;
    
    int which_buf_slot_ = 0;
    std::array<Token, 2> token_buf_;

    // these are automatically incremented and decremented by the get_token()
    unsigned int indent_level_ = 0;
    unsigned int parenthese_level_ = 0;
    unsigned int curly_brace_level_ = 0;
    unsigned int angle_bracket_level_ = 0;

    bool finished_ = false;
    bool program_valid_ = true;
};

#endif