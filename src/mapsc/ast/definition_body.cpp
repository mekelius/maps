#include "definition_body.hh"

#include "mapsc/compilation_state.hh"
#include "mapsc/ast/ast_store.hh"

namespace Maps {

DefinitionBody::DefinitionBody(DefinitionHeader* header, LetDefinitionValue value)
:header_(header), value_(value) {}


DefinitionBody* create_let_definition(AST_Store& ast_store, const std::string& name, 
    LetDefinitionValue value, const SourceLocation& location) {

    return *ast_store.allocate_definition({name, location}, value)->body_;
}

DefinitionBody* create_let_definition(AST_Store& ast_store, 
    LetDefinitionValue value, const SourceLocation& location) {

    // !!! names will clash
    return create_let_definition(ast_store, "anonymous_definition", value, location);
}

DefinitionBody* create_let_definition(AST_Store& ast_store, const std::string& name, 
    Expression* value, const SourceLocation& location) {

    return *ast_store.allocate_definition({name, value->type, location}, value)->body_;
}

DefinitionBody* create_let_definition(AST_Store& ast_store, Expression* value, 
    const SourceLocation& location) {

    // !!! names will clash
    return *ast_store.allocate_definition({"anonymous_definition", value->type, location}, value)->body_;
}

} // namespace Maps