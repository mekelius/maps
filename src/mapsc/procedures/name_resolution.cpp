#include "name_resolution.hh"

#include <cassert>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/reference.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/logging.hh"

#include "mapsc/source_location.hh"
#include "mapsc/types/type_store.hh"

using std::optional, std::nullopt;


namespace Maps {

using Log = LogInContext<LogContext::name_resolution>;

namespace {

optional<DefinitionHeader*> lookup_identifier(std::span<const Scope* const> ct_scopes, 
    const_Scopes rt_scopes, const std::string& name) {
    
    for (auto scope: ct_scopes) {
        auto definition = scope->get_identifier(name);
        if (definition)
            return definition;
    }

    for (auto scope: rt_scopes) {
        auto definition = scope->get_identifier(name);
        if (definition)
            return definition;
    }   

    return nullopt;
}

optional<DefinitionHeader*> lookup_identifier(const Scope& ct_scope, const_Scopes rt_scopes, 
    const std::string& name) {

    return lookup_identifier(std::array{&ct_scope}, rt_scopes, name);
}

optional<DefinitionHeader*> lookup_identifier(CompilationState& state, const_Scopes rt_scopes, 
    const std::string& name) {

    return lookup_identifier(*state.builtins_, rt_scopes, name);
}

bool resolve_identifier(CompilationState& state, const_Scopes scopes, 
    Expression* expression) {

    auto definition = lookup_identifier(state, scopes, expression->string_value());

    if (!definition) {
        Log::error(expression->location) << "Unknown identifier: " << expression->string_value() << Endl;
        return false;
    }

    convert_to_reference(*expression, *definition);

    if (expression->expression_type == ExpressionType::known_value_reference)
        if (!convert_by_value_substitution(*expression))
            return false;

    return true;
}

bool resolve_operator(CompilationState& state, const_Scopes scopes, 
    Expression* expression) {

    auto definition = lookup_identifier(state, scopes, expression->string_value());

    if (!definition) {
        Log::error(expression->location) << "Unknown operator: " << expression->string_value() << Endl;
        return false;
    }

    if (!(*definition)->is_operator()) {
        Log::compiler_error(expression->location) << 
            "resolve_operator called with non-operator: " << **definition;
        return false;
    }

    convert_to_operator_reference(*expression, dynamic_cast<Operator*>(*definition));
    return true;
}

bool resolve_type_identifier(CompilationState& state, Expression* expression) {
    Log::debug_extra(expression->location) << "Attempting to resolve " << *expression << Endl;
    
    // check builtins
    std::optional<const Type*> type = state.types_->get(expression->string_value());
    if (!type) {
        Log::error(expression->location) << 
            "Unkown type identifier: " << expression->string_value() << Endl;
        return false;
    }
    Log::debug_extra(expression->location) << "Found type " << **type << Endl;
    
    expression->expression_type = ExpressionType::type_reference;
    expression->value = *type;
    return true;
}

} // anonymous namespace


// Replaces all identifiers and operators with references to the correct definitions
bool resolve_identifiers(CompilationState& state, const_Scopes scopes, 
    std::vector<Expression*>& unresolved_identifiers) {

    Log::debug_extra(NO_SOURCE_LOCATION) << "Resolving identifiers" << Endl;

    for (Expression* expression: unresolved_identifiers) {
        switch (expression->expression_type) {
            case ExpressionType::identifier:
                // assert(ast_->builtins_.identifier_exists(expression->string_value()) 
                //     && "Builtin identifier passed to layer2");
                if (!resolve_identifier(state, scopes, expression))
                    return false;
                break;
            case ExpressionType::operator_identifier:
                if (!resolve_operator(state, scopes, expression))
                    return false;
                break;

            case ExpressionType::type_identifier:
                if (!resolve_type_identifier(state, expression))
                    return false;
                break;

            default:
                Log::error(expression->location) <<
                    "Unexpected expression type in unresolved_identifiers_and_operators";
                assert(false 
                    && "Unexpected expression type in unresolved_identifiers_and_operators");
                return false;
        }
    }
    return true;
}

bool resolve_identifiers(CompilationState& state, const Scope& scope,
    std::vector<Expression*>& unresolved_identifiers) {

    return resolve_identifiers(state,
        std::array{&scope}, 
        unresolved_identifiers);
}

} // namespace Maps