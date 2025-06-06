#ifndef __AST_REFERENCE_HH
#define __AST_REFERENCE_HH

#include <optional>
#include <string>

#include "mapsc/ast/scope.hh"
#include "mapsc/ast/operator.hh"

namespace Maps {

class DefinitionHeader;
class DefinitionBody;
struct SourceLocation;
class Type;
class AST_Store;
class Operator;

std::optional<Expression*> create_reference(AST_Store& store, const Scope* scope, 
    const std::string& name, const SourceLocation& location);
Expression* create_reference(AST_Store& store, 
    const DefinitionHeader* callee, const SourceLocation& location);
Expression* create_reference(AST_Store& store, 
    DefinitionBody* callee, const SourceLocation& location);

Expression* create_type_reference(AST_Store& store, 
    const Type* type, const SourceLocation& location);
Expression create_operator_reference(
    const Operator* callee, const SourceLocation& location);
Expression* create_operator_reference(AST_Store& store, 
    const Operator* callee, const SourceLocation& location);

std::optional<Expression*> create_type_operator_reference(AST_Store& store, 
    const std::string& name, const Type* type, const SourceLocation& location);

void convert_to_reference(Expression& expression, const DefinitionHeader* callee);
void convert_to_operator_reference(Expression& expression, const Operator* callee);
[[nodiscard]] bool convert_by_value_substitution(Expression& expression);

// Accepts operator ref and the various partially applied binary operators
Operator::Precedence get_operator_precedence(const Expression& operator_ref, bool from_left = true);

} // namespace Maps

#endif