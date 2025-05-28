#ifndef __TERMED_EXPRESSION_HH
#define __TERMED_EXPRESSION_HH

#include <string>

#include "mapsc/ast/scope.hh"
#include "expression.hh"

namespace Maps {

class AST_Store;

Expression* create_termed(AST_Store& store, 
    std::vector<Expression*>&& terms, RT_Definition* context, const SourceLocation& location);
Expression* create_termed_testing(AST_Store& store, 
    std::vector<Expression*>&& terms, const SourceLocation& location, bool top_level = true);


} // namespace Maps

#endif