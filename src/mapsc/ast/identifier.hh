#ifndef __EXPRESSIONS_IDENTIFIERS_HH
#define __EXPRESSIONS_IDENTIFIERS_HH

#include <string>

#include "mapsc/ast/scope.hh"

namespace Maps {

class AST_Store;
struct Expression;

Expression* create_identifier(AST_Store& store, Scope* scope,
        const std::string& value, const SourceLocation& location);
Expression* create_operator_identifier(AST_Store& store, Scope* scope, 
        const std::string& value, const SourceLocation& location);
Expression* create_type_identifier(AST_Store& store, 
        const std::string& value, const SourceLocation& location);
Expression* create_type_operator_identifier(AST_Store& store, 
        const std::string& value, const SourceLocation& location);


} // namespace Maps

#endif