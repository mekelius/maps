#include "builtins.hh"

#include "ast.hh"
#include "types.hh"

void init_builtins(AST::AST& ast) {
    init_builtin_callables(ast);
    init_builtin_operators(ast);
}

void init_builtin_operators(AST::AST& ast) {
    AST::Expression* plus = ast.create_expression(AST::ExpressionType::builtin_operator,
        {0, 0}, AST::create_binary_operator_type(AST::Number, AST::Number, AST::Number, true, 1, AST::Associativity::both));
    ast.builtin_operators_.create_identifier("+", {0,0});

    AST::Expression* minus = ast.create_expression(AST::ExpressionType::builtin_operator,
        {0, 0}, AST::create_binary_operator_type(AST::Number, AST::Number, AST::Number, true, 1, AST::Associativity::left));
    ast.builtin_operators_.create_identifier("-", {0,0});

    AST::Expression* times = ast.create_expression(AST::ExpressionType::builtin_operator,
        {0, 0}, AST::create_binary_operator_type(AST::Number, AST::Number, AST::Number, true, 2, AST::Associativity::both));
    ast.builtin_operators_.create_identifier("*", {0,0});

    // TODO: subset types here
    AST::Expression* divide = ast.create_expression(AST::ExpressionType::builtin_operator,
        {0, 0}, AST::create_binary_operator_type(AST::Number, AST::Number, AST::Number, true, 3, AST::Associativity::left));
    ast.builtin_operators_.create_identifier("/", {0,0});
}

void init_builtin_callables(AST::AST& ast) {
    AST::Expression* print = ast.create_expression(AST::ExpressionType::builtin_function, 
        {0, 0}, AST::Void);
    
    // print->type = AST::create_function_type(AST::Void, {AST::String});

    if (!ast.builtins_.create_identifier("print", print, {0,0})) {
        assert(false && "couldn't create builtin: print");
    }
}