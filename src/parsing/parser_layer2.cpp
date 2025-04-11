#include "parser_layer2.hh"

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
                // TODO: assert not a builtin
                resolve_identifier(expression);
                break;
            case AST::ExpressionType::unresolved_operator:
                // TODO: assert not a native operator
                // TODO: resolve it
                break;

            default:
                assert(false 
                    && "Unexpected expression type in AST.unresolved_identifiers_and_operators");
        }
    }

    ast_->unresolved_identifiers_and_operators = {};
}

void ParserLayer2::resolve_identifier(AST::Expression* expression) {
    std::optional<AST::Identifier*> callable = ast_->global_.get_identifier(expression->string_value());

    if (!callable) {

    }
}

void ParserLayer2::resolve_operator(AST::Expression* expression) {
    assert(false && "ParserLayer2::resolve_operator not implemented");
}

void ParserLayer2::declare_invalid() {
    ast_->valid = false;
}
    
void ParserLayer2::select_expression(AST::Expression* expression) {

}

AST::Expression* ParserLayer2::parse_termed_expression() {

}