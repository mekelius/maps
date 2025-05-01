#ifndef __NAME_RESOLUTION_HH
#define __NAME_RESOLUTION_HH

#include "mapsc/ast/ast.hh"

namespace Maps {

bool resolve_identifiers(AST& ast);
bool resolve_identifier(AST& ast, Expression* expression);
bool resolve_operator(AST& ast, Expression* expression);
bool resolve_type_identifier(AST& ast, Expression* expression);

} //namespace Maps

#endif