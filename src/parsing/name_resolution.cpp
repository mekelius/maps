#include "name_resolution.hh"

#include <cassert>

#include "../logging.hh"

using Logging::log_error, Logging::log_info;

// Replaces all identifiers and operators with references to the correct callables
void resolve_identifiers(Maps::AST& ast) {
    for (Maps::Expression* expression: ast.unresolved_identifiers_and_operators) {
        switch (expression->expression_type) {
            case Maps::ExpressionType::identifier:
                // assert(ast_->builtins_.identifier_exists(expression->string_value()) 
                //     && "Builtin identifier passed to layer2");
                resolve_identifier(ast, expression);
                break;
            case Maps::ExpressionType::operator_e:
                resolve_operator(ast, expression);
                break;

            default:
                assert(false 
                    && "Unexpected expression type in AST.unresolved_identifiers_and_operators");
        }
    }

    ast.unresolved_identifiers_and_operators = {};
}

// this should be scope's responsibility
void resolve_identifier(Maps::AST& ast, Maps::Expression* expression) {
    // check builtins
    std::optional<Maps::Callable*> builtin = ast.builtins_scope_->get_identifier(expression->string_value());
    if (builtin) {
        log_info(expression->location, "Parsed built-in", Logging::MessageType::parser_debug_terminal);
        expression->expression_type = Maps::ExpressionType::reference;
        expression->type = (*builtin)->get_type();
        expression->value = *builtin;
        return;
    }

    std::optional<Maps::Callable*> callable = ast.globals_->get_identifier(expression->string_value());

    if (!callable) {
        log_error(expression->location, "unknown identifier: " + expression->string_value());
        ast.declare_invalid();
        return;
    }

    expression->expression_type = Maps::ExpressionType::reference;
    expression->type = (*callable)->get_type();
    expression->value = *callable;
    return;
}

void resolve_operator(Maps::AST& ast, Maps::Expression* expression) {
    std::optional<Maps::Callable*> builtin = ast.builtins_scope_->get_identifier(expression->string_value());
    if (builtin) {
        assert((*builtin)->is_operator() && "during name resolution: encountered a builtin operator_e that pointed to not an operator");

        log_info(expression->location, "Parsed built-in operator", Logging::MessageType::parser_debug_terminal);
        expression->expression_type = Maps::ExpressionType::operator_ref;
        expression->value = *builtin;
        expression->type = (*builtin)->get_type();
        return;
    }

    // TODO:user-defined operators
    // std::optional<AST::Callable*> callable = ast_->global_operators_.get_identifier(expression->string_value());

    // if (!callable) {
        log_error(expression->location, "unknown operator: " + expression->string_value());
        ast.declare_invalid();
        return;
    // }

    // expression->expression_type = AST::ExpressionType::identifier;
    // expression->type = (*callable)->get_type();
    // return;
}