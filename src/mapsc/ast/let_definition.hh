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

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    Scope* outer_scope, std::string name, LetDefinitionValue body_value, bool is_top_level, SourceLocation location);

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    Scope* outer_scope, std::string name, const Type* type, bool is_top_level, SourceLocation location);

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    const Type* type, SourceLocation location);

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    std::string name, LetDefinitionValue value, SourceLocation location);

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    LetDefinitionValue value, const SourceLocation& location);

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    std::string name, Expression* value, SourceLocation location);

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store,
    Expression* value, SourceLocation location);

} // namespace Maps

#endif