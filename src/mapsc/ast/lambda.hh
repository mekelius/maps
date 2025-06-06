#ifndef __LAMBDA_EXPRESSION_HH
#define __LAMBDA_EXPRESSION_HH

#include "expression.hh"

namespace Maps {

class DefinitionBody;
class AST_Store;

std::tuple<Expression*, DefinitionBody*> create_const_lambda(CompilationState& state, 
    Expression* expression, std::span<const Type* const> param_types, 
    const SourceLocation& location, bool is_pure = true);
std::tuple<Expression*, DefinitionBody*> create_const_lambda(CompilationState& state, 
    KnownValue value, std::span<const Type* const> param_types, 
    const SourceLocation& location, bool is_pure = true);

} // namespace Maps

#endif