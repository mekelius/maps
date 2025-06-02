#ifndef __LET_DEFINITION_HH
#define __LET_DEFINITION_HH

#include <optional>
#include <string_view>
#include <variant>

#include "common/std_visit_helper.hh"
#include "common/maps_datatypes.h"

#include "mapsc/ast/scope.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/ast_store.hh"

namespace Maps {

DefinitionBody* create_let_definition(AST_Store& ast_store, Scope* outer_scope, const std::string& name,
    LetDefinitionValue body_value, bool is_top_level, const SourceLocation& location) {

    const Type* type = &Hole;

    return *ast_store.allocate_definition(
        {DefinitionType::let_definition, name, type, location}, 
        body_value)->body_;
}

DefinitionBody* create_let_definition(AST_Store& ast_store, Scope* outer_scope, const std::string& name, 
    const Type* type, bool is_top_level, const SourceLocation& location) {

    return *ast_store.allocate_definition(
        DefinitionHeader{DefinitionType::let_definition, name, type, location}, 
        LetDefinitionValue{Undefined{}})->body_;
}

DefinitionBody* create_let_definition(AST_Store& ast_store, const Type* type, 
    const SourceLocation& location) {

    return *ast_store.allocate_definition(
        {DefinitionType::let_definition, "anonymous_definition", type, location})->body_;
}

DefinitionBody* create_let_definition(AST_Store& ast_store, const std::string& name, 
    LetDefinitionValue value, const SourceLocation& location) {

    return *ast_store.allocate_definition(
        {DefinitionType::let_definition, name, location}, value)->body_;
}

DefinitionBody* create_let_definition(AST_Store& ast_store, 
    LetDefinitionValue value, const SourceLocation& location) {

    // !!! names will clash
    return create_let_definition(ast_store, "anonymous_definition", value, location);
}

DefinitionBody* create_let_definition(AST_Store& ast_store, const std::string& name, 
    Expression* value, const SourceLocation& location) {

    return *ast_store.allocate_definition(
        {DefinitionType::let_definition, name, value->type, location}, value)->body_;
}

// DefinitionBody* create_let_definition(AST_Store& ast_store, Expression* value, 
//     const SourceLocation& location) {

//     // !!! names will clash
//     return *ast_store.allocate_definition(
//         {DefinitionType::let_definition, "anonymous_definition", value->type, location}, value)->body_;
// }

DefinitionBody* create_let_definition(AST_Store& ast_store,
    Expression* value, const SourceLocation& location) {
 
    return *ast_store.allocate_definition(
        {DefinitionType::let_definition, "anonymous definition", value->type, location},
        value)->body_;
}

} // namespace Maps

#endif