#ifndef __NAME_RESOLUTION_HH
#define __NAME_RESOLUTION_HH

#include "mapsc/ast/ast.hh"

namespace Maps {

bool resolve_identifiers(AST_Store& ast);
bool resolve_identifier(AST_Store& ast, Expression* expression);
bool resolve_operator(AST_Store& ast, Expression* expression);
bool resolve_type_identifier(AST_Store& ast, Expression* expression);

} //namespace Maps

#endif