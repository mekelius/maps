#ifndef __LET_DEFINITION_HH
#define __LET_DEFINITION_HH

#include <optional>
#include <string_view>
#include <variant>

#include "common/std_visit_helper.hh"
#include "common/maps_datatypes.h"

#include "mapsc/ast/scope.hh"
#include "mapsc/ast/definition.hh"

namespace Maps {

class Type;
struct Expression;
struct Statement;
class AST_Store;
class CompilationState;
struct SourceLocation;

DefinitionBody* create_let_definition(AST_Store& ast_store, Scope* outer_scope, const std::string& name,
    LetDefinitionValue body_value, bool is_top_level, const SourceLocation& location);

DefinitionBody* create_let_definition(AST_Store& ast_store, Scope* outer_scope, const std::string& name, 
    const Type* type, bool is_top_level, const SourceLocation& location);

DefinitionBody* create_let_definition(AST_Store& ast_store, const Type* type, 
    const SourceLocation& location);

DefinitionBody* create_let_definition(AST_Store& ast_store, const std::string& name, 
    LetDefinitionValue value, const SourceLocation& location);

DefinitionBody* create_let_definition(AST_Store& ast_store, 
    LetDefinitionValue value, const SourceLocation& location);

DefinitionBody* create_let_definition(AST_Store& ast_store, const std::string& name, 
    Expression* value, const SourceLocation& location);

DefinitionBody* create_let_definition(AST_Store& ast_store,
    Expression* value, const SourceLocation& location);

} // namespace Maps

#endif