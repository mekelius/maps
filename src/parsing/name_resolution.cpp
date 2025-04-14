#include "name_resolution.hh"

using Logging::log_error, Logging::log_info;


void resolve_identifiers(AST::AST& ast) {
    for (AST::Expression* expression: ast.unresolved_identifiers_and_operators) {
        switch (expression->expression_type) {
            case AST::ExpressionType::unresolved_identifier:
                // assert(ast_->builtins_.identifier_exists(expression->string_value()) 
                //     && "Builtin identifier passed to layer2");
                resolve_identifier(ast, expression);
                break;
            case AST::ExpressionType::unresolved_operator:
                resolve_operator(ast, expression);
                break;

            default:
                assert(false 
                    && "Unexpected expression type in AST.unresolved_identifiers_and_operators");
        }
    }

    ast.unresolved_identifiers_and_operators = {};
}

void resolve_identifier(AST::AST& ast, AST::Expression* expression) {
    std::optional<AST::Callable*> builtin = ast.builtins_.get_identifier(expression->string_value());
    if (builtin) {
        log_info(expression->location, "Parsed built-in", Logging::MessageType::parser_debug_terminal);
        expression->expression_type = AST::ExpressionType::builtin_function;
        expression->type = (*builtin)->get_type();
        return;
    }

    std::optional<AST::Callable*> callable = ast.globals_.get_identifier(expression->string_value());

    if (!callable) {
        log_error(expression->location, "unknown identifier: " + expression->string_value());
        ast.declare_invalid();
        return;
    }

    expression->expression_type = AST::ExpressionType::identifier;
    expression->type = (*callable)->get_type();
    return;
}

void resolve_operator(AST::AST& ast, AST::Expression* expression) {
    std::optional<AST::Callable*> builtin = ast.builtin_operators_.get_identifier(expression->string_value());
    if (builtin) {
        log_info(expression->location, "Parsed built-in operator", Logging::MessageType::parser_debug_terminal);
        expression->expression_type = AST::ExpressionType::builtin_operator;
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