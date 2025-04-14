#ifndef __NAME_RESOLUTION_HH
#define __NAME_RESOLUTION_HH

#include "../lang/ast.hh"

void resolve_identifiers(AST::AST& ast);
void resolve_identifier(AST::AST& ast, AST::Expression* expression);
void resolve_operator(AST::AST& ast, AST::Expression* expression);

#endif