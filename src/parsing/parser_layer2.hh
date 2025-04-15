#ifndef __PARSER_LAYER_2_HH
#define __PARSER_LAYER_2_HH

/**
 * Parser layer 2 is responsible for:
 *  - resolving rest of the names
 *  - parsing "termed expressions" i.e. binary and unary operators and access expressions
 *  - parsing mapping literals i.e. lists, enums, dictionaries etc.
 *  - doing type inference
 */

#include "../lang/ast.hh"

constexpr unsigned int MAX_OPERATOR_PRECEDENCE = 1000;

// TODO: these things don't really logically belong together in any way
//       i.e. shoulnd't be a class
class ParserLayer2 {
  public:
    ParserLayer2(AST::AST* ast, Pragma::Pragmas* pragmas);

    void run();

  private:
    
    // Selects a termed expression to parse. That expressions terms become the tokenstream
    void select_expression(AST::Expression* expression);
    AST::Expression* get_term();
    
    // caller must first check that we aren't at expression end by calling at_expression_end 
    AST::Expression* peek() const;
    void shift();
    bool at_expression_end() const;
    bool parse_stack_reduced() const;
    
    AST::Expression* parse_termed_expression();
    void parse_tied_operator();
    
    // state functioms
    void initial_identifier_state();
    void initial_value_state();
    void initial_operator_state();
    void pre_binary_operator_state();
    void post_binary_operator_state();
    void arg_list_state();
    void unary_operators_state();
    
    AST::AST* ast_;
    Pragma::Pragmas* pragmas_;

    AST::Expression* current_expression_;
    std::vector<AST::Expression*> current_terms_;
    AST::Expression* current_term_;
    AST::Expression* next_term_;
    std::vector<AST::Expression*>::iterator next_term_it_;

    std::vector<AST::Expression*> parse_stack_;
    unsigned int previous_operator_precedence_ = MAX_OPERATOR_PRECEDENCE;
};

#endif