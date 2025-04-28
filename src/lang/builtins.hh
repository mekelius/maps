#ifndef __BUILTINS_HH
#define __BUILTINS_HH

#include "ast/ast.hh"

void init_builtins(Maps::AST& ast);
void init_builtin_callables(Maps::AST& ast);
void init_builtin_operators(Maps::AST& ast);

#endif