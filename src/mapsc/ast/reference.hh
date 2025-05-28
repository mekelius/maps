#ifndef __AST_REFERENCE_HH
#define __AST_REFERENCE_HH

#include <optional>
#include <string>

#include "mapsc/ast/scope.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/source.hh"

namespace Maps {

Expression* create_reference(AST_Store& store, 
    Definition* callee, const SourceLocation& location);
Expression* create_type_reference(AST_Store& store, 
    const Type* type, const SourceLocation& location);
Expression create_operator_reference(
    Definition* callee, const SourceLocation& location);
Expression* create_operator_reference(AST_Store& store, 
    Definition* callee, const SourceLocation& location);

template <class T>
std::optional<Expression*> create_reference(AST_Store& store, const Scope_T<T> scope, 
    const std::string& name, const SourceLocation& location) {
        if (auto definition = scope.get_identifier(name))
            return create_reference(store, *definition, location);

        return std::nullopt;
    }

std::optional<Expression*> create_type_operator_reference(AST_Store& store, 
    const std::string& name, const Type* type, const SourceLocation& location);

} // namespace Maps

#endif