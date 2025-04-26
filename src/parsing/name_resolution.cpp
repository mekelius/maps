#include "name_resolution.hh"

#include <cassert>

#include "../logging.hh"

using Logging::log_error, Logging::log_info;

namespace Maps {

// Replaces all identifiers and operators with references to the correct callables
bool resolve_identifiers(AST& ast) {
    for (Expression* expression: ast.unresolved_identifiers_and_operators) {
        switch (expression->expression_type) {
            case ExpressionType::identifier:
                // assert(ast_->builtins_.identifier_exists(expression->string_value()) 
                //     && "Builtin identifier passed to layer2");
                resolve_identifier(ast, expression);
                break;
            case ExpressionType::operator_identifier:
                resolve_operator(ast, expression);
                break;

            case ExpressionType::type_identifier:
                resolve_type_identifier(ast, expression);
                break;

            default:
                log_error(expression->location, 
                    "Unexpected expression type in AST.unresolved_identifiers_and_operators");
                assert(false 
                    && "Unexpected expression type in AST.unresolved_identifiers_and_operators");
                return false;
        }
    }

    if (!ast.is_valid)
        return false;

    ast.unresolved_identifiers_and_operators = {};
    return true;
}

// this should be scope's responsibility
bool resolve_identifier(AST& ast, Expression* expression) {
    // check builtins
    std::optional<Callable*> builtin = ast.builtins_scope_->get_identifier(expression->string_value());
    if (builtin) {
        log_info(expression->location, "Parsed built-in", Logging::MessageType::parser_debug_terminal);
        expression->expression_type = ExpressionType::reference;
        expression->type = (*builtin)->get_type();
        expression->value = *builtin;
        return true;
    }

    std::optional<Callable*> callable = ast.globals_->get_identifier(expression->string_value());

    if (!callable) {
        log_error(expression->location, "unknown identifier: " + expression->string_value());
        ast.declare_invalid();
        return false;
    }

    expression->expression_type = ExpressionType::reference;
    expression->type = (*callable)->get_type();
    expression->value = *callable;
    return true;
}

bool resolve_type_identifier(AST& ast, Expression* expression) {
    // check builtins
    std::optional<const Type*> type = ast.types_->get(expression->string_value());
    if (!type) {
        log_error(expression->location, "unkown type identifier: " + expression->string_value());
        ast.declare_invalid();
        return false;
    }
    
    expression->expression_type = ExpressionType::type_reference;
    expression->value = *type;
    return true;
}

bool resolve_operator(AST& ast, Expression* expression) {
    std::optional<Callable*> builtin = ast.builtins_scope_->get_identifier(expression->string_value());
    if (builtin) {
        if (!(*builtin)->is_operator()) {
            log_error("during name resolution: encountered a builtin operator_e that pointed to not an operator");
            assert(false && 
                "during name resolution: encountered a builtin operator_e that pointed to not an operator");
            ast.declare_invalid();
            return false;
        }

        log_info(expression->location, "Parsed built-in operator", Logging::MessageType::parser_debug_terminal);
        expression->expression_type = ExpressionType::operator_reference;
        expression->value = *builtin;
        expression->type = (*builtin)->get_type();
        return true;
    }

    log_error(expression->location, "unknown operator: " + expression->string_value());
    ast.declare_invalid();
    return true;
}

} // namespace Maps