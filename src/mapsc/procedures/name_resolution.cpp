#include "name_resolution.hh"

#include <cassert>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "mapsc/ast/callable.hh"
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

optional<Callable*> lookup_identifier(const Scopes& scopes, const std::string& name) {
    auto [ct_scopes, rt_scopes] = scopes;

    for (auto scope: ct_scopes) {
        auto callable = scope->get_identifier(name);
        if (callable)
            return callable;
    }

    for (auto scope: rt_scopes) {
        auto callable = scope->get_identifier(name);
        if (callable)
            return callable;
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

    auto callable = lookup_identifier(scopes, expression->string_value());

    if (!callable) {
        Log::error("unknown identifier: " + expression->string_value(), expression->location);
        return false;
    }

    expression->expression_type = ExpressionType::reference;
    expression->type = (*callable)->get_type();
    expression->value = *callable;
    return true;
}

bool resolve_type_identifier(CompilationState& state, const Scopes& scopes, 
    Expression* expression) {
    
    // check builtins
    std::optional<const Type*> type = state.types_->get(expression->string_value());
    if (!type) {
        Log::error("unkown type identifier: " + expression->string_value(), expression->location);
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

    auto callable = lookup_identifier(scopes, expression->string_value());

    if (!callable) {
        Log::error("unknown operator: " + expression->string_value(), expression->location);
        return false;
    }

    expression->convert_to_operator_reference(*callable);
    return true;
}

} // anonymous namespace


// Replaces all identifiers and operators with references to the correct callables
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

bool resolve_identifiers(CompilationState& state, const std::vector<RT_Scope*>& scopes, 
    std::vector<Expression*>& unresolved_identifiers) {

    return resolve_identifiers(state, Scopes{{}, scopes}, unresolved_identifiers);
}

bool resolve_identifiers(CompilationState& state, const std::vector<CT_Scope*>& scopes, 
    std::vector<Expression*>& unresolved_identifiers) {

    return resolve_identifiers(state, Scopes{scopes, {}}, unresolved_identifiers);
}


} // namespace Maps