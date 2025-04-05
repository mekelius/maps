#ifndef __PARSER_HH
#define __PARSER_HH

#include <istream>
#include <memory>
#include <array>
#include <ostream>
#include <vector>
#include <unordered_map>

#include "lexer.hh"
#include "ast.hh"

// First attempt at a parser. Parses tokens directly into the llvm context
class Parser {
  public:
    Parser(StreamingLexer* lexer, std::ostream* error_stream);
    
    std::unique_ptr<AST::AST> run();
  private:
    // gets the next token from the lexer and stores it in current_token_
    Token get_token();
    // declare the program invalid. Parsing may still continue, but no final output should be produced
    void declare_invalid();
    std::unique_ptr<AST::AST> finalize_parsing();

    // Token expect_token(predicate, error_message);
    void print_error(const std::string& message) const;
    void print_error(const std::string& location, const std::string& message) const;
    void print_info(const std::string& message) const;
    void print_info(const std::string& location, const std::string& message) const;
    void print_parsing_complete() const;

    // ---- IDENTIFIERS -----

    bool identifier_exists(const std::string& identifier) const;
    void create_identifier(const std::string& identifier, AST::Expression* expression);

    AST::Expression* parse_expression();
    AST::Expression* parse_call_expression();
    void parse_let_statement();
    
    StreamingLexer* lexer_;
    std::ostream* errs_;
    std::unique_ptr<AST::AST> ast_;
    
    Token current_token_;
    bool finished_ = false;
    bool program_valid_ = true;
    std::unordered_map<std::string, AST::Expression*> identifiers_;
};

#endif