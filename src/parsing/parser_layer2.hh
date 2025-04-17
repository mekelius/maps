#ifndef __PARSER_LAYER_2_HH
#define __PARSER_LAYER_2_HH

/**
 * Parser layer 2 is responsible for:
 *  - parsing "termed expressions" i.e. binary and unary operators and access expressions
 *  - parsing mapping literals i.e. lists, enums, dictionaries etc.
 */

#include "../lang/ast.hh"

constexpr unsigned int MAX_OPERATOR_PRECEDENCE = 1000;
constexpr unsigned int MIN_OPERATOR_PRECEDENCE = 0;

class TermedExpressionParser {
  public:
    TermedExpressionParser(AST::AST* ast, AST::Expression* expression);
    // parses the expression in-place
    void run();

  private:  
    // advances the input stream without putting the term on the parse stack
    AST::Expression* get_term();
        
    // caller must first check that we aren't at expression end by calling at_expression_end 
    AST::Expression* peek() const;
    // caller must be sure that the parse_stack isn't empty
    AST::Expression* current_term() const;

    // advances the input stream by one and pushes the term onto the parse stack
    void shift();

    // pops an expression from the parse stack, returns nullopt if parse stack empty
    std::optional<AST::Expression*> pop_term();

    bool at_expression_end() const;
    bool parse_stack_reduced() const;
    
    AST::Expression* parse_termed_expression();

    AST::Expression* handle_termed_sub_expression(AST::Expression* expression);
    
    // state functioms
    void initial_identifier_state();
    void initial_value_state();
    void initial_operator_state();
    void initial_function_state();
    
    // binary operators
    void post_binary_operator_state();
    void compare_precedence_state();
    void reduce_operator_left();
    
    void unary_operators_state();

    // calls/access operations
    bool is_acceptable_next_arg(AST::Callable* callee, 
        const std::vector<AST::Expression*>& args, AST::Expression* next_arg);

    void call_expression_state();
    AST::Expression* handle_arg_state(AST::Callable* callee, const std::vector<AST::Expression*>& args);

    AST::AST* ast_;

    AST::Expression* expression_;
    std::vector<AST::Expression*>* expression_terms_;
    std::vector<AST::Expression*>::iterator next_term_it_;

    std::vector<AST::Expression*> parse_stack_ = {};
    std::vector<unsigned int> precedence_stack_ = {MIN_OPERATOR_PRECEDENCE};
};

// basically a small wrapper that creates TermedExpressionParser for each unparsed termed expression
// in the ast and runs them
class ParserLayer2 {
  public:
    ParserLayer2(AST::AST* ast, Pragma::Pragmas* pragmas);
    void run();

  private:
    // Selects a termed expression to parse. That expressions terms become the tokenstream
    void select_expression(AST::Expression* expression);
    AST::AST* ast_;
    Pragma::Pragmas* pragmas_;
};

#endif