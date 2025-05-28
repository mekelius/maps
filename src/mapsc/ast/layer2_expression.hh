#ifndef __TERMED_EXPRESSION_HH
#define __TERMED_EXPRESSION_HH

#include <vector>

namespace Maps {
    
struct SourceLocation;
class AST_Store;
struct Expression;
class RT_Definition;

Expression* create_layer2_expression(AST_Store& store, 
    std::vector<Expression*>&& terms, RT_Definition* context, const SourceLocation& location);
Expression* create_layer2_expression_testing(AST_Store& store, 
    std::vector<Expression*>&& terms, const SourceLocation& location, bool top_level = true);

} // namespace Maps

#endif