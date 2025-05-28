#ifndef __MISC_EXPRESSION_HH
#define __MISC_EXPRESSION_HH

#include <string>

#include "mapsc/ast/scope.hh"
#include "expression.hh"

namespace Maps {

class AST_Store;

Expression* create_valueless(AST_Store& store, 
    ExpressionType expression_type, const SourceLocation& location);

Expression* create_minus_sign(AST_Store& store, const SourceLocation& location);

Expression* create_user_error(AST_Store& store, const SourceLocation& location);
Expression* create_compiler_error(AST_Store& store, const SourceLocation& location);


} // namespace Maps

#endif