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

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    Scope* outer_scope, std::string name, LetDefinitionValue body_value, 
    bool is_top_level, SourceLocation location) {

    const Type* type = &Unknown; // !!!

    return ast_store.allocate_definition(
        {DefinitionType::let_definition, std::move(name), type, std::move(location)}, 
        body_value);
}

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    Scope* outer_scope, std::string name, const Type* type, bool is_top_level, 
    SourceLocation location) {

    return ast_store.allocate_definition(
        RT_DefinitionHeader{DefinitionType::let_definition, std::move(name), type, std::move(location)}, 
        LetDefinitionValue{Undefined{}});
}

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    const Type* type, SourceLocation location) {

    return ast_store.allocate_definition(
        {DefinitionType::let_definition, "anonymous_definition", type, std::move(location)}, Undefined{});
}

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    std::string name, LetDefinitionValue value, SourceLocation location) {

    return ast_store.allocate_definition(
        {DefinitionType::let_definition, std::move(name), std::move(location)}, value);
}

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    LetDefinitionValue value, SourceLocation location) {

    // !!! names will clash
    return create_let_definition(ast_store, "anonymous_definition", value, std::move(location));
}

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store, 
    std::string name, Expression* value, SourceLocation location) {

    return ast_store.allocate_definition(
        {DefinitionType::let_definition, std::move(name), value->type, std::move(location)}, value);
}

// DefinitionBody* create_let_definition(AST_Store& ast_store, Expression* value, 
//     const SourceLocation& location) {

//     // !!! names will clash
//     return *ast_store.allocate_definition(
//         {DefinitionType::let_definition, "anonymous_definition", value->type, location}, value)->body_;
// }

std::pair<DefinitionHeader*, DefinitionBody*> create_let_definition(AST_Store& ast_store,
    Expression* value, SourceLocation location) {
 
    return ast_store.allocate_definition(
        {DefinitionType::let_definition, "anonymous definition", value->type, std::move(location)},
        value);
}

} // namespace Maps

#endif