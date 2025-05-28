#ifndef __CALL_EXPRESSION_HH
#define __CALL_EXPRESSION_HH

#include "expression.hh"

namespace Maps {

class AST_Store;

std::optional<Expression*> create_call(CompilationState& state, 
    Definition* callee, std::vector<Expression*>&& args, const SourceLocation& location);

std::optional<Expression*> create_partial_binop_call(CompilationState& state, 
    Definition* callee, Expression* lhs, Expression* rhs, const SourceLocation& location);

// not implemented
std::optional<Expression*> create_partial_binop_call_both(CompilationState& state,
    Definition* lhs, Expression* lambda, Definition* rhs, const SourceLocation& location);

Expression* create_partially_applied_minus(AST_Store& store, 
    Expression* rhs, const SourceLocation& location);

Expression* create_missing_argument(AST_Store& store, 
    const Type* type, const SourceLocation& location);

// For example partial binop call, currently a no-op
void convert_to_partial_call(Expression& expression);
// expect to be a partially applied minus
[[nodiscard]] bool convert_to_partial_binop_minus_call_left(CompilationState& state, 
    Expression& expression);
[[nodiscard]] bool convert_to_unary_minus_call(CompilationState& state, Expression& expression);
void convert_nullary_reference_to_call(Expression& expression);
[[nodiscard]] bool convert_partially_applied_minus_to_arg(CompilationState& state, 
    Expression& expression, const Type* param_type);

} // namespace Maps

#endif