#include "name_resolution.hh"

#include <cassert>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/logging.hh"

#include "mapsc/source.hh"
#include "mapsc/types/type_store.hh"

using std::optional, std::nullopt;


namespace Maps {

using Log = LogInContext<LogContext::name_resolution>;

namespace {

optional<Definition*> lookup_identifier(const Scopes& scopes, const std::string& name) {
    auto [ct_scopes, rt_scopes] = scopes;

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

// this should be scope's responsibility
bool resolve_identifier(CompilationState& state, const Scopes& scopes, 
    Expression* expression) {
    
    // check builtins
    if (auto builtin = state.builtins_->get_identifier(expression->string_value())) {
        Log::debug_extra("Parsed built-in", expression->location);
        expression->expression_type = ExpressionType::reference;
        expression->type = (*builtin)->get_type();
        expression->value = *builtin;
        return true;
    }

    auto definition = lookup_identifier(scopes, expression->string_value());

    if (!definition) {
        Log::error("unknown identifier: " + expression->string_value(), expression->location);
        return false;
    }

    expression->expression_type = ExpressionType::reference;
    expression->type = (*definition)->get_type();
    expression->value = *definition;
    return true;
}

bool resolve_type_identifier(CompilationState& state, const Scopes& scopes, 
    Expression* expression) {
    
    // check builtins
    std::optional<const Type*> type = state.types_->get(expression->string_value());
    if (!type) {
        Log::error("unkown type identifier: " + expression->string_value(), 
            expression->location);
        return false;
    }
    
    expression->expression_type = ExpressionType::type_reference;
    expression->value = *type;
    return true;
}

bool resolve_operator(CompilationState& state, const Scopes& scopes, 
    Expression* expression) {
    
    if (auto builtin = state.builtins_->get_identifier(expression->string_value())) {
        if (!(*builtin)->is_operator()) {
            Log::compiler_error("during name resolution: encountered a builtin operator_e \
that pointed to not an operator", expression->location);
            assert(false && 
                "during name resolution: encountered a builtin operator_e that pointed to \
not an operator");
            return false;
        }

        Log::debug_extra("Resolved built-in operator", expression->location);
        
        expression->convert_to_operator_reference(*builtin);
        return true;
    }

    auto definition = lookup_identifier(scopes, expression->string_value());

    if (!definition) {
        Log::error("unknown operator: " + expression->string_value(), expression->location);
        return false;
    }

    expression->convert_to_operator_reference(*definition);
    return true;
}

} // anonymous namespace


// Replaces all identifiers and operators with references to the correct definitions
bool resolve_identifiers(CompilationState& state, const Scopes& scopes, 
    std::vector<Expression*>& unresolved_identifiers) {
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
                if (!resolve_type_identifier(state, scopes, expression))
                    return false;
                break;

            default:
                Log::error("Unexpected expression type in unresolved_identifiers_and_operators", 
                    expression->location);
                assert(false 
                    && "Unexpected expression type in unresolved_identifiers_and_operators");
                return false;
        }
    }
    return true;
}

bool resolve_identifiers(CompilationState& state, std::span<const RT_Scope* const> scopes, 
    std::vector<Expression*>& unresolved_identifiers) {

    return resolve_identifiers(state, {{}, scopes}, unresolved_identifiers);
}

bool resolve_identifiers(CompilationState& state, std::span<const CT_Scope* const> scopes, 
    std::vector<Expression*>& unresolved_identifiers) {

    return resolve_identifiers(state, {scopes, {}}, unresolved_identifiers);
}

bool resolve_identifiers(CompilationState& state, const CT_Scope& ct_scope, 
    const RT_Scope& rt_scope, std::vector<Expression*>& unresolved_identifiers) {

    return resolve_identifiers(state, {
        std::array<const CT_Scope* const, 1>{&ct_scope}, 
        std::array<const RT_Scope* const, 1>{&rt_scope}}, 
        unresolved_identifiers);
}

bool resolve_identifiers(CompilationState& state, const RT_Scope& scope,
    std::vector<Expression*>& unresolved_identifiers) {

    return resolve_identifiers(state, {
        std::array<const CT_Scope* const, 1>{state.builtins_}, 
        std::array<const RT_Scope* const, 2>{&state.globals_, &scope}}, 
        unresolved_identifiers);
}

bool resolve_identifiers(CompilationState& state, 
    std::vector<Expression*>& unresolved_identifiers) {

    return resolve_identifiers(state, Scopes{
        std::array<const CT_Scope* const, 1>{state.builtins_}, 
        std::array<const RT_Scope* const, 1>{&state.globals_}}, 
        unresolved_identifiers);
}


} // namespace Maps