#ifndef __CALL_EXPRESSION_HH
#define __CALL_EXPRESSION_HH

#include "expression.hh"

namespace Maps {

class AST_Store;
class DefinitionHeader;

std::optional<Expression*> create_call(CompilationState& state, const DefinitionHeader* callee, 
    std::vector<Expression*>&& args, SourceLocation location);

std::optional<Expression*> create_call(CompilationState& state, 
    DefinitionBody* callee, std::vector<Expression*>&& args, SourceLocation location);

std::optional<Expression*> create_partial_binop_call(CompilationState& state, 
    const Operator* callee, Expression* lhs, Expression* rhs, SourceLocation location);

// not implemented
std::optional<Expression*> create_partial_binop_call_both(CompilationState& state,
    const Operator* lhs, Expression* lambda, const Operator* rhs, SourceLocation location);

Expression* create_partially_applied_minus(AST_Store& store, Expression* rhs, SourceLocation location);
Expression* create_missing_argument(AST_Store& store, const Type* type, SourceLocation location);

// For example partial binop call, currently a no-op
void convert_to_partial_call(Expression& expression);
// expect to be a partially applied minus
[[nodiscard]] bool convert_to_partial_binop_call_left(CompilationState& state, 
    Expression& expression);
[[nodiscard]] bool convert_to_unary_minus_call(CompilationState& state, Expression& expression);
void convert_nullary_reference_to_call(Expression& expression);
[[nodiscard]] bool convert_partially_applied_minus_to_arg(CompilationState& state, 
    Expression& expression, const Type* param_type);

[[nodiscard]] std::optional<Expression*> complete_partial_binop_call_left(CompilationState& state, 
    Expression& partial_binop_call, Expression& arg);

} // namespace Maps

#endif