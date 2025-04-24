#ifndef __NAME_RESOLUTION_HH
#define __NAME_RESOLUTION_HH

#include "../lang/ast.hh"

void resolve_identifiers(Maps::AST& ast);
void resolve_identifier(Maps::AST& ast, Maps::Expression* expression);
void resolve_operator(Maps::AST& ast, Maps::Expression* expression);

#endif