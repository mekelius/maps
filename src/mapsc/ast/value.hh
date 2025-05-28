#ifndef __VALUE_EXPRESSION_HH
#define __VALUE_EXPRESSION_HH

#include <string>

#include "mapsc/ast/scope.hh"
#include "expression.hh"

namespace Maps {

class AST_Store;

Expression* create_string_literal(AST_Store& store, 
    const std::string& value, const SourceLocation& location);
Expression* create_numeric_literal(AST_Store& store, 
    const std::string& value, const SourceLocation& location);
Expression* create_known_value(CompilationState& state, KnownValue value,
    const SourceLocation& location);
std::optional<Expression*> create_known_value(CompilationState& state, KnownValue value, 
    const Type* type, const SourceLocation& location);


} // namespace Maps

#endif