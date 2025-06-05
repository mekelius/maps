#ifndef __NAME_RESOLUTION_HH
#define __NAME_RESOLUTION_HH

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
#include "mapsc/ast/value.hh"
#include "mapsc/ast/builtin.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/logging.hh"

#include "mapsc/source_location.hh"
#include "mapsc/types/type_store.hh"

namespace Maps {

class CompilationState;
struct Expression;

template<size_t size_p>
std::optional<const DefinitionHeader*> lookup_definition(const Scope& scope, std::string_view name,
    BuiltinExternalScope<size_p> builtin_externals) {
    
    if (auto definition = scope.get_identifier(name))
        return definition;

    if (auto definition = builtin_externals.get_identifier(name))
        return definition;

    return std::nullopt;
}

template<size_t size_p>
std::optional<const BuiltinValue*> lookup_value(std::string_view name,
    BuiltinValueScope<size_p> builtin_values) {
    
    return builtin_values.get_identifier(name);
}

template<typename BuiltinScopes>
bool resolve_identifier(const Scope& scope, Expression& expression, BuiltinScopes builtins) {
    using Log = LogInContext<LogContext::name_resolution>;

    Log::debug_extra(expression.location) << "Resolving " << expression << Endl;

    auto& [builtin_externals, builtin_values] = builtins;
    auto definition = lookup_definition(scope, expression.string_value(), builtin_externals);

    if (definition) {
        Log::debug_extra(expression.location) << "Found definition " << **definition << Endl;

        convert_to_reference(expression, *definition);

        if (expression.expression_type == ExpressionType::known_value_reference)
            if (!convert_by_value_substitution(expression))
                return false;

        return true;
    }

    // Try to find a value
    auto value = lookup_value(expression.string_value(), builtin_values);

    if (!value) {
        Log::error(expression.location) << "Unknown identifier: " << expression.string_value() << Endl;
        return false;
    }

    Log::debug_extra(expression.location) << "Found known value " << *value << Endl;

    convert_to_known_value(expression, **value);
    return true;
}

template<typename BuiltinScopes>
bool resolve_operator(const Scope& scope, Expression& expression, BuiltinScopes builtins) {
    using Log = LogInContext<LogContext::name_resolution>;

    auto& [builtin_externals, _] = builtins;
    auto definition = lookup_definition(scope, expression.string_value(), builtin_externals);

    if (!definition) {
        Log::error(expression.location) << "Unknown operator: " << expression.string_value() << Endl;
        return false;
    }

    if (!(*definition)->is_operator()) {
        Log::compiler_error(expression.location) << 
            "resolve_operator called with non-operator: " << **definition;
        return false;
    }

    convert_to_operator_reference(expression, dynamic_cast<const Operator*>(*definition));
    return true;
}

inline bool resolve_type_identifier(CompilationState& state, Expression& expression) {
    using Log = LogInContext<LogContext::name_resolution>;

    Log::debug_extra(expression.location) << "Attempting to resolve " << expression << Endl;
    
    std::optional<const Type*> type = state.types_->get(expression.string_value());
    if (!type) {
        Log::error(expression.location) << 
            "Unkown type identifier: " << expression.string_value() << Endl;
        return false;
    }
    Log::debug_extra(expression.location) << "Found type " << **type << Endl;
    
    expression.expression_type = ExpressionType::type_reference;
    expression.value = *type;
    return true;
}

// Replaces all identifiers and operators with references to the correct definitions
template<typename BuiltinScopes>
bool resolve_identifiers(CompilationState& state, const Scope& scope, 
    std::vector<Expression*>& unresolved_identifiers, BuiltinScopes builtins) {

    using Log = LogInContext<LogContext::name_resolution>;

    Log::debug_extra(NO_SOURCE_LOCATION) << "Resolving identifiers" << Endl;

    for (Expression* expression: unresolved_identifiers) {
        switch (expression->expression_type) {
            case ExpressionType::identifier:
                if (!resolve_identifier(scope, *expression, builtins))
                    return false;
                break;
            case ExpressionType::operator_identifier:
                if (!resolve_operator(scope, *expression, builtins))
                    return false;
                break;

            case ExpressionType::type_identifier:
                if (!resolve_type_identifier(state, *expression))
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

inline bool resolve_identifiers(CompilationState& state, const Scope& scope, 
    std::vector<Expression*>& unresolved_identifiers) {

    return resolve_identifiers(
        state, scope, unresolved_identifiers, builtins);
}

} //namespace Maps

#endif