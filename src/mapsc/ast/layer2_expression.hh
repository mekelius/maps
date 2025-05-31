#ifndef __TERMED_EXPRESSION_HH
#define __TERMED_EXPRESSION_HH

#include <vector>

namespace Maps {
    
struct SourceLocation;
struct Expression;
class AST_Store;
class Scope;

Expression* create_layer2_expression(AST_Store& store, 
    std::vector<Expression*>&& terms, Scope* context, const SourceLocation& location);
Expression* create_layer2_expression_testing(AST_Store& store, 
    std::vector<Expression*>&& terms, const SourceLocation& location, bool top_level = true);

} // namespace Maps

#endif