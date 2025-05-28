#ifndef __MISC_EXPRESSION_HH
#define __MISC_EXPRESSION_HH

#include <string>

namespace Maps {

class AST_Store;
struct SourceLocation;
struct Expression;

Expression* create_minus_sign(AST_Store& store, const SourceLocation& location);

Expression* create_user_error(AST_Store& store, const SourceLocation& location);
Expression* create_compiler_error(AST_Store& store, const SourceLocation& location);

} // namespace Maps

#endif