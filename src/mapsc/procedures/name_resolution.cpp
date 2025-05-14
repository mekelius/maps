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
#include "mapsc/loglevel_defs.hh"
#include "mapsc/source.hh"
#include "mapsc/types/type_store.hh"

using Maps::GlobalLogger::log_error, Maps::GlobalLogger::log_info;

namespace Maps {

class Type;

// Replaces all identifiers and operators with references to the correct callables
bool resolve_identifiers(CompilationState& state) {
    for (Expression* expression: state.unresolved_identifiers_) {
        switch (expression->expression_type) {
            case ExpressionType::identifier:
                // assert(ast_->builtins_.identifier_exists(expression->string_value()) 
                //     && "Builtin identifier passed to layer2");
                resolve_identifier(state, expression);
                break;
            case ExpressionType::operator_identifier:
                resolve_operator(state, expression);
                break;

            case ExpressionType::type_identifier:
                resolve_type_identifier(state, expression);
                break;

            default:
                log_error("Unexpected expression type in unresolved_identifiers_and_operators", 
                    expression->location);
                assert(false 
                    && "Unexpected expression type in unresolved_identifiers_and_operators");
                return false;
        }
    }

    if (!state.is_valid)
        return false;

    state.unresolved_identifiers_ = {};
    return true;
}

// this should be scope's responsibility
bool resolve_identifier(CompilationState& state, Expression* expression) {
    // check builtins
    if (auto builtin = state.builtins_->get_identifier(expression->string_value())) {
        log_info("Parsed built-in", MessageType::parser_debug_terminal, expression->location);
        expression->expression_type = ExpressionType::reference;
        expression->type = (*builtin)->get_type();
        expression->value = *builtin;
        return true;
    }

    std::optional<Callable*> callable = state.globals_->get_identifier(expression->string_value());

    if (!callable) {
        log_error("unknown identifier: " + expression->string_value(), expression->location);
        state.declare_invalid();
        return false;
    }

    expression->expression_type = ExpressionType::reference;
    expression->type = (*callable)->get_type();
    expression->value = *callable;
    return true;
}

bool resolve_type_identifier(CompilationState& state, Expression* expression) {
    // check builtins
    std::optional<const Type*> type = state.types_->get(expression->string_value());
    if (!type) {
        log_error("unkown type identifier: " + expression->string_value(), expression->location);
        state.declare_invalid();
        return false;
    }
    
    expression->expression_type = ExpressionType::type_reference;
    expression->value = *type;
    return true;
}

bool resolve_operator(CompilationState& state, Expression* expression) {
    if (auto builtin = state.builtins_->get_identifier(expression->string_value())) {
        if (!(*builtin)->is_operator()) {
            log_error("during name resolution: encountered a builtin operator_e that pointed to not an operator");
            assert(false && 
                "during name resolution: encountered a builtin operator_e that pointed to not an operator");
            state.declare_invalid();
            return false;
        }

        log_info("Parsed built-in operator", MessageType::parser_debug_terminal, expression->location);
        expression->expression_type = ExpressionType::operator_reference;
        expression->value = *builtin;
        expression->type = (*builtin)->get_type();
        return true;
    }

    log_error("unknown operator: " + expression->string_value(), expression->location);
    state.declare_invalid();
    return false;
}

} // namespace Maps