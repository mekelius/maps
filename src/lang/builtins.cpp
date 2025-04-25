#include "builtins.hh"

#include "ast.hh"
#include "type.hh"

void init_builtins(Maps::AST& ast) {
    init_builtin_callables(ast);
    init_builtin_operators(ast);
}

void init_builtin_operators(Maps::AST& ast) {
    ast.create_builtin_binary_operator("+", *ast.types_->get_function_type(Maps::Number, 
        {&Maps::Number, &Maps::Number}), 1, Maps::Associativity::left /* Maps::Associativity::both*/);

    ast.create_builtin_binary_operator("-", *ast.types_->get_function_type(Maps::Number, 
        {&Maps::Number, &Maps::Number}), 1, Maps::Associativity::left);

    ast.create_builtin_binary_operator("*", *ast.types_->get_function_type(Maps::Number, 
        {&Maps::Number, &Maps::Number}), 2, Maps::Associativity::left /* Maps::Associativity::both*/);

    // TODO: subset types here
    ast.create_builtin_binary_operator("/", *ast.types_->get_function_type(Maps::Number, 
        {&Maps::Number, &Maps::Number}), 3, Maps::Associativity::left);
}

void init_builtin_callables(Maps::AST& ast) {
    ast.create_builtin("print",
        *ast.types_->get_function_type(Maps::Void, {&Maps::String}));

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Maps::Void, {&Maps::Int}));

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Maps::Void, {&Maps::Float}));

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Maps::Void, {&Maps::Boolean}));
}