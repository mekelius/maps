#ifndef __PARSER_LAYER_2_IMPLEMENTATION_HH
#define __PARSER_LAYER_2_IMPLEMENTATION_HH

#include <optional>
#include <string>
#include <vector>

#include "mapsc/source.hh"
#include "mapsc/ast/operator.hh"

/**
 * Parser layer 2 is responsible for:
 *  - parsing "termed expressions" i.e. binary and unary operators and access expressions
 *  - parsing mapping literals i.e. lists, enums, dictionaries etc.
 */

namespace Maps {

struct Expression;
class Definition;
class CompilationState;
class AST_Store;

class TermedExpressionParser {
public:
    TermedExpressionParser(CompilationState* compilation_state, Expression* expression);
    // parses the expression in-place
    bool run();

private:  
    // advances the input stream without putting the term on the parse stack
    Expression* get_term();
        
    // caller must first check that we aren't at expression end by calling at_expression_end 
    Expression* peek() const;
    // caller must be sure that the parse_stack isn't empty
    Expression* current_term() const;

    // helper to get the precedence of an operator on top of the stack
    // caller must check that there is an operator at the top of the stack
    Operator::Precedence peek_precedence() const;

    // advances the input stream by one and pushes the term onto the parse stack
    void shift();

    // pops an expression from the parse stack, returns nullopt if parse stack empty
    std::optional<Expression*> pop_term();

    void fail(const std::string& message, SourceLocation location);

    bool at_expression_end() const;
    bool parse_stack_reduced() const;
    
    std::optional<Expression*> parse_termed_expression();

    Expression* handle_termed_sub_expression(Expression* expression);
    
    // state functioms
    void initial_reference_state();
    void initial_call_state();
    void initial_partial_call_state();
    void initial_value_state();
    void initial_binary_operator_state();
    void initial_prefix_operator_state();
    void initial_postfix_operator_state();
    void initial_minus_sign_state();
    void initial_partially_applied_minus_state();
    void initial_partial_binop_call_right_state();
    void initial_partial_binop_call_left_state();
    void initial_partial_binop_call_both_state();

    void reference_state();
    void call_state();
    void value_state();
    void prefix_operator_state();

    void partial_binop_call_standoff_state();
    void post_binary_operator_state();
    void compare_precedence_state();

    // reductions
    void reduce_binop_call();
    void reduce_minus_sign_to_unary_minus_call();
    void reduce_to_partial_binop_call_left();
    void reduce_to_partial_binop_call_right();
    void reduce_to_partial_binop_call_both();
    void reduce_to_unary_minus_ref();
    void reduce_prefix_operator();
    void reduce_postfix_operator();
    void reduce_partially_applied_minus();
    void reduce_partial_call();

    // gotos
    void initial_goto();

    // callee ref is used as the default location
    void push_partial_call(Expression* callee_ref, const std::vector<Expression*>& args);
    void push_partial_call(Expression* callee_ref, const std::vector<Expression*>& args,
        SourceLocation location);
    void add_to_partial_call_and_push(Expression* partial_call, const std::vector<Expression*>& args, 
        SourceLocation location);
    // convenience function that creates an unary call expression and pushes it onto the parse stack
    void push_unary_operator_call(Expression* operator_ref, Expression* value);
    void apply_type_declaration(Expression* type_declaration, Expression* value);
    
    void initial_type_reference_state();
    void initial_type_constructor_state();

    // calls/access operations
    bool is_acceptable_next_arg(Definition* callee, 
      const std::vector<Expression*>& args/*, Expression* next_arg*/);
      
    void call_expression_state();
    void partial_call_state();
    void deferred_call_state();

    Expression* handle_arg_state(Definition* callee, const std::vector<Expression*>& args);

    // creating minus refs
    Expression* binary_minus_ref(SourceLocation location);
    Expression* unary_minus_ref(SourceLocation location);

    Expression* const expression_;
    std::vector<Expression*>* expression_terms_;
    std::vector<Expression*>::iterator next_term_it_;
    CompilationState* const compilation_state_;
    AST_Store* const ast_store_;

    bool possibly_type_expression_ = true;
    std::vector<Expression*> parse_stack_ = {};
    std::vector<Operator::Precedence> precedence_stack_ = {Operator::MIN_PRECEDENCE};

    bool success_ = true;
};

} // namespace Maps

#endif