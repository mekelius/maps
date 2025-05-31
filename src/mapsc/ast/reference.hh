#ifndef __AST_REFERENCE_HH
#define __AST_REFERENCE_HH

#include <optional>
#include <string>

#include "mapsc/ast/scope.hh"

namespace Maps {

class DefinitionHeader;
struct SourceLocation;
class Type;
class AST_Store;
class Operator;

std::optional<Expression*> create_reference(AST_Store& store, const Scope* scope, 
    const std::string& name, const SourceLocation& location);
Expression* create_reference(AST_Store& store, 
    DefinitionHeader* callee, const SourceLocation& location);
Expression* create_type_reference(AST_Store& store, 
    const Type* type, const SourceLocation& location);
Expression create_operator_reference(
    Operator* callee, const SourceLocation& location);
Expression* create_operator_reference(AST_Store& store, 
    Operator* callee, const SourceLocation& location);

std::optional<Expression*> create_type_operator_reference(AST_Store& store, 
    const std::string& name, const Type* type, const SourceLocation& location);

void convert_to_reference(Expression& expression, DefinitionHeader* callee);
void convert_to_operator_reference(Expression& expression, Operator* callee);
[[nodiscard]] bool convert_by_value_substitution(Expression& expression);

} // namespace Maps

#endif