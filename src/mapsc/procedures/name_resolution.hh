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
#include "mapsc/compilation_state.hh"
#include "mapsc/logging.hh"

#include "mapsc/source_location.hh"
#include "mapsc/types/type_store.hh"

namespace Maps {

class CompilationState;
struct Expression;

template<size_t s>
std::optional<const DefinitionHeader*> lookup_identifier(const Scope& scope, std::string_view name,
    const BuiltinScope<s>& builtins) {
    
    if (auto definition = scope.get_identifier(name))
        return definition;

    if (auto definition = builtins.get_identifier(name))
        return definition;

    return std::nullopt;
}

template<size_t s>
bool resolve_identifier(const Scope& scope, Expression* expression, const BuiltinScope<s>& builtins) {
    using Log = LogInContext<LogContext::name_resolution>;

    auto definition = lookup_identifier(scope, expression->string_value(), builtins);

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

template<size_t s>
bool resolve_operator(const Scope& scope, Expression* expression, const BuiltinScope<s>& builtins_) {
    using Log = LogInContext<LogContext::name_resolution>;

    auto definition = lookup_identifier(scope, expression->string_value(), builtins_);

    if (!definition) {
        Log::error(expression->location) << "Unknown operator: " << expression->string_value() << Endl;
        return false;
    }

    if (!(*definition)->is_operator()) {
        Log::compiler_error(expression->location) << 
            "resolve_operator called with non-operator: " << **definition;
        return false;
    }

    convert_to_operator_reference(*expression, dynamic_cast<const Operator*>(*definition));
    return true;
}

inline bool resolve_type_identifier(CompilationState& state, Expression* expression) {
    using Log = LogInContext<LogContext::name_resolution>;

    Log::debug_extra(expression->location) << "Attempting to resolve " << *expression << Endl;
    
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

// Replaces all identifiers and operators with references to the correct definitions
template<size_t s>
bool resolve_identifiers(CompilationState& state, const Scope& scope, 
    std::vector<Expression*>& unresolved_identifiers, const BuiltinScope<s>& builtins) {
    using Log = LogInContext<LogContext::name_resolution>;

    Log::debug_extra(NO_SOURCE_LOCATION) << "Resolving identifiers" << Endl;

    for (Expression* expression: unresolved_identifiers) {
        switch (expression->expression_type) {
            case ExpressionType::identifier:
                if (!resolve_identifier(scope, expression, builtins))
                    return false;
                break;
            case ExpressionType::operator_identifier:
                if (!resolve_operator(scope, expression, builtins))
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

inline bool resolve_identifiers(CompilationState& state, const Scope& scope, 
    std::vector<Expression*>& unresolved_identifiers) {

    return resolve_identifiers(state, scope, unresolved_identifiers, builtins);
}

} //namespace Maps

#endif