#ifndef __BUILTINS_HH
#define __BUILTINS_HH

#include "ast.hh"

void init_builtins(AST::AST& ast);
void init_builtin_callables(AST::AST& ast);
void init_builtin_operators(AST::AST& ast);

#endif