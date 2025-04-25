#ifndef __PARSER_LAYER_2_HH
#define __PARSER_LAYER_2_HH

/**
 * Parser layer 2 is responsible for:
 *  - parsing "termed expressions" i.e. binary and unary operators and access expressions
 *  - parsing mapping literals i.e. lists, enums, dictionaries etc.
 */

#include "../lang/pragma.hh"
#include "../lang/ast.hh"

class TermedExpressionParser {
  public:
    TermedExpressionParser(Maps::AST* ast, Maps::Expression* expression);
    // parses the expression in-place
    void run();

  private:  
    // advances the input stream without putting the term on the parse stack
    Maps::Expression* get_term();
        
    // caller must first check that we aren't at expression end by calling at_expression_end 
    Maps::Expression* peek() const;
    // caller must be sure that the parse_stack isn't empty
    Maps::Expression* current_term() const;

    // helper to get the precedence of an operator on top of the stack
    // caller must check that there is an operator at the top of the stack
    Maps::Precedence peek_precedence() const;

    // advances the input stream by one and pushes the term onto the parse stack
    void shift();

    // pops an expression from the parse stack, returns nullopt if parse stack empty
    std::optional<Maps::Expression*> pop_term();

    bool at_expression_end() const;
    bool parse_stack_reduced() const;
    
    Maps::Expression* parse_termed_expression();

    Maps::Expression* handle_termed_sub_expression(Maps::Expression* expression);
    
    // state functioms
    void initial_identifier_state();
    void initial_value_state();
    void initial_operator_state();
    
    // binary operators
    void post_binary_operator_state();
    void compare_precedence_state();
    void reduce_operator_left();
    
    void unary_operators_state();
    
    // calls/access operations
    bool is_acceptable_next_arg(Maps::Callable* callee, 
      const std::vector<Maps::Expression*>& args, Maps::Expression* next_arg);
      
    void call_expression_state();
    void partial_call_state();
    Maps::Expression* handle_arg_state(Maps::Callable* callee, const std::vector<Maps::Expression*>& args);

    Maps::AST* ast_;

    Maps::Expression* expression_;
    std::vector<Maps::Expression*>* expression_terms_;
    std::vector<Maps::Expression*>::iterator next_term_it_;

    std::vector<Maps::Expression*> parse_stack_ = {};
    std::vector<unsigned int> precedence_stack_ = {Maps::MIN_OPERATOR_PRECEDENCE};
};

// basically a small wrapper that creates TermedExpressionParser for each unparsed termed expression
// in the ast and runs them
class ParserLayer2 {
  public:
    ParserLayer2(Maps::AST* ast, Pragma::Pragmas* pragmas);
    void run();

  private:
    // Selects a termed expression to parse. That expressions terms become the tokenstream
    void select_expression(Maps::Expression* expression);
    Maps::AST* ast_;
    Pragma::Pragmas* pragmas_;
};

#endif