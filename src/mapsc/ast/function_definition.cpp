#include "function_definition.hh"

#include <optional>
#include <string_view>
#include <variant>

#include "common/maps_datatypes.h"

#include "mapsc/compilation_state.hh"
#include "mapsc/log_format.hh"
#include "mapsc/source.hh"

#include "mapsc/types/type_defs.hh"

#include "mapsc/ast/scope.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/definition_body.hh"
#include "mapsc/ast/ast_store.hh"

namespace Maps {

DefinitionBody* function_definition(CompilationState& state, 
    const ParameterList& parameter_list, Scope* inner_scope, LetDefinitionValue value, 
    bool is_top_level, const SourceLocation& location) {

    std::vector<const Type*> param_types{};

    for (auto param: parameter_list)
        param_types.push_back(param->get_type());

    if (std::holds_alternative<Expression*>(value)) {
        return *state.ast_store_->allocate_definition(
            DefinitionHeader{MAPS_INTERNALS_PREFIX + "anonymous_function", location}, value)->body_;
    }

    return *state.ast_store_->allocate_definition(
        DefinitionHeader{MAPS_INTERNALS_PREFIX + "anonymous_function", location}, value)->body_;
}

DefinitionBody* function_definition(CompilationState& state, 
    const ParameterList& parameter_list, Scope* inner_scope, bool is_top_level, 
    const SourceLocation& location) {
    
    return function_definition(state, parameter_list, inner_scope, Undefined{}, is_top_level, location);
}

DefinitionBody* function_definition(CompilationState& state, const std::string& name, 
    Expression* value, const SourceLocation& location) {
    
    return *state.ast_store_->allocate_definition(DefinitionHeader{name, value->type, location}, 
        value)->body_;
}

DefinitionBody* create_nullary_function_definition(AST_Store& ast_store, TypeStore& types, 
    Expression* value, bool is_pure, const SourceLocation& location) {

    return *ast_store.allocate_definition(
        DefinitionHeader{"testing_nullary_function", types.get_function_type(value->type, {}, is_pure), 
            location}, value)->body_;
}

Parameter* create_parameter(AST_Store& ast_store, const std::string& name, const Type* type, 
    const SourceLocation& location) {

    return ast_store.allocate_parameter(Parameter{name, type, location});
}

Parameter* create_parameter(AST_Store& ast_store, const std::string& name, 
    const SourceLocation& location) {

    return create_parameter(ast_store, name, &Hole, location);
}

Parameter* create_discarded_parameter(AST_Store& ast_store, const Type* type, 
    const SourceLocation& location) {

    return ast_store.allocate_parameter(Parameter{type, location});
}

Parameter* create_discarded_parameter(AST_Store& ast_store, const SourceLocation& location) {
    return create_discarded_parameter(ast_store, &Hole, location);
}

} // namespace Maps
