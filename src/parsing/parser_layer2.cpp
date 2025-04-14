#include <cassert>

#include "parser_layer2.hh"

using Logging::log_error, Logging::log_info;

ParserLayer2::ParserLayer2(AST::AST* ast, Pragma::Pragmas* pragmas)
: ast_(ast), pragmas_(pragmas) {
}

void ParserLayer2::run() {
    resolve_identifiers();

    // infer types

}

void ParserLayer2::resolve_identifiers() {
    for (AST::Expression* expression: ast_->unresolved_identifiers_and_operators) {
        switch (expression->expression_type) {
            case AST::ExpressionType::unresolved_identifier:
                // assert(ast_->builtins_.identifier_exists(expression->string_value()) 
                //     && "Builtin identifier passed to layer2");
                resolve_identifier(expression);
                break;
            case AST::ExpressionType::unresolved_operator:
                resolve_operator(expression);
                break;

            default:
                assert(false 
                    && "Unexpected expression type in AST.unresolved_identifiers_and_operators");
        }
    }

    ast_->unresolved_identifiers_and_operators = {};
}

void ParserLayer2::resolve_identifier(AST::Expression* expression) {
    std::optional<AST::Callable*> builtin = ast_->builtins_.get_identifier(expression->string_value());
    if (builtin) {
        log_info(expression->location, "Parsed built-in", Logging::MessageType::parser_debug_terminal);
        expression->expression_type = AST::ExpressionType::builtin_function;
        expression->type = (*builtin)->get_type();
        return;
    }

    std::optional<AST::Callable*> callable = ast_->globals_.get_identifier(expression->string_value());

    if (!callable) {
        log_error(expression->location, "unknown identifier: " + expression->string_value());
        declare_invalid();
        return;
    }

    expression->expression_type = AST::ExpressionType::identifier;
    expression->type = (*callable)->get_type();
    return;
}

void ParserLayer2::resolve_operator(AST::Expression* expression) {
    std::optional<AST::Callable*> builtin = ast_->builtin_operators_.get_identifier(expression->string_value());
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
        declare_invalid();
        return;
    // }

    // expression->expression_type = AST::ExpressionType::identifier;
    // expression->type = (*callable)->get_type();
    // return;
}

void ParserLayer2::declare_invalid() {
    ast_->is_valid = false;
}
    
void ParserLayer2::select_expression(AST::Expression* expression) {

}

AST::Expression* ParserLayer2::parse_termed_expression() {

}