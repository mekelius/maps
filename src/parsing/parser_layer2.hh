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

// TODO: these things don't really logically belong together in any way
//       i.e. shoulnd't be a class
class ParserLayer2 {
  public:
    ParserLayer2(AST::AST* ast, Pragma::Pragmas* pragmas);

    void run();

  private:
    
    // Selects a termed expression to parse. That expressions terms become the tokenstream
    void select_expression(AST::Expression* expression);
    void reduce_termed_expressions();
    AST::Expression* reduce_termed_expression(AST::Expression* expression);
    
    AST::AST* ast_;
    Pragma::Pragmas* pragmas_;
};

#endif