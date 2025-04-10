#include "parser_layer2.hh"

ParserLayer2::ParserLayer2(AST::AST* ast): ast_(ast) {
}

void ParserLayer2::run() {
    resolve_identifiers();

    // infer types

}

void ParserLayer2::resolve_identifiers() {

}

void ParserLayer2::declare_invalid() {

}
    
void ParserLayer2::select_expression(AST::Expression* expression) {

}

AST::Expression* ParserLayer2::parse_termed_expression() {

}