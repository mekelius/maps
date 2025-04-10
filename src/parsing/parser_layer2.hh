#ifndef __PARSER_LAYER2_HH
#define __PARSER_LAYER2_HH

/**
 * Parser layer 2 is responsible for:
 *  - resolving rest of the names
 *  - parsing "termed expressions" i.e. binary and unary operators and access expressions
 *  - parsing mapping literals i.e. lists, enums, dictionaries etc.
 *  - doing type inference
 */

#include "../lang/ast.hh"

class ParserLayer2 {
    using Term = AST::Expression;
    using TermList = std::vector<Term*>;
    using TermIterator = TermList::iterator;
  public:
    ParserLayer2(AST::AST* ast);

    void run();

  private:
    // TODO: refactor logging

    // walks the tree and tries to resolve any unresolved identifiers
    void resolve_identifiers();

    // declares the tree invalid
    void declare_invalid();
    
    // Selects a termed expression to parse. That expressions terms become the tokenstream
    void select_expression(AST::Expression* expression);
    AST::Expression* parse_termed_expression();
    
    TermList current_terms_;
    TermIterator current_term_;

    AST::AST* ast_;
};

#endif