#ifndef __CALL_EXPRESSION_HH
#define __CALL_EXPRESSION_HH

#include <string>

#include "mapsc/ast/scope.hh"
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

} // namespace Maps

#endif