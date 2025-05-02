#ifndef __PARSER_LAYER_2_HH
#define __PARSER_LAYER_2_HH

/**
 * Parser layer 2 is responsible for:
 *  - parsing "termed expressions" i.e. binary and unary operators and access expressions
 *  - parsing mapping literals i.e. lists, enums, dictionaries etc.
 */

#include "mapsc/pragma.hh"
#include "mapsc/ast/ast.hh"

namespace Maps {

class TermedExpressionParser {
public:
    TermedExpressionParser(AST* ast, Expression* expression);
    // parses the expression in-place
    void run();

private:  
    // advances the input stream without putting the term on the parse stack
    Expression* get_term();
        
    // caller must first check that we aren't at expression end by calling at_expression_end 
    Expression* peek() const;
    // caller must be sure that the parse_stack isn't empty
    Expression* current_term() const;

    // helper to get the precedence of an operator on top of the stack
    // caller must check that there is an operator at the top of the stack
    Precedence peek_precedence() const;

    // advances the input stream by one and pushes the term onto the parse stack
    void shift();

    // pops an expression from the parse stack, returns nullopt if parse stack empty
    std::optional<Expression*> pop_term();

    bool at_expression_end() const;
    bool parse_stack_reduced() const;
    
    Expression* parse_termed_expression();

    Expression* handle_termed_sub_expression(Expression* expression);
    
    // state functioms
    void initial_reference_state();
    void initial_value_state();
    void initial_operator_state();
    
    // binary operators
    void post_binary_operator_state();
    void compare_precedence_state();
    void reduce_operator_left();
    
    void initial_type_reference_state();
    void initial_type_constructor_state();

    // calls/access operations
    bool is_acceptable_next_arg(Callable* callee, 
      const std::vector<Expression*>& args/*, Expression* next_arg*/);
      
    void call_expression_state();
    void partial_call_state();
    Expression* handle_arg_state(Callable* callee, const std::vector<Expression*>& args);

    AST* ast_;

    Expression* expression_;
    std::vector<Expression*>* expression_terms_;
    std::vector<Expression*>::iterator next_term_it_;

    bool possibly_type_expression_ = true;
    std::vector<Expression*> parse_stack_ = {};
    std::vector<unsigned int> precedence_stack_ = {MIN_OPERATOR_PRECEDENCE};
};

// basically a small wrapper that creates TermedExpressionParser for each unparsed termed expression
// in the ast and runs them
class ParserLayer2 {
public:
    ParserLayer2(AST* ast, PragmaStore* pragmas);
    void run();

private:

    // Selects a termed expression to parse. That expressions terms become the tokenstream
    void select_expression(Expression* expression);
    AST* ast_;
    PragmaStore* pragmas_;
};

} // namespace Maps

#endif