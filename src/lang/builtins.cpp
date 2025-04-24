#include "builtins.hh"

#include "ast.hh"
#include "type.hh"

void init_builtins(Maps::AST& ast) {
    init_builtin_callables(ast);
    init_builtin_operators(ast);
}

void init_builtin_operators(Maps::AST& ast) {
    using Maps::BuiltinType;

    ast.create_builtin(BuiltinType::builtin_operator, "+",
        *ast.types_->get_binary_operator_type(Maps::Number, Maps::Number, Maps::Number, 1, Maps::Associativity::both));

    ast.create_builtin(BuiltinType::builtin_operator, "-", 
        *ast.types_->get_binary_operator_type(Maps::Number, Maps::Number, Maps::Number, 1, Maps::Associativity::left));

    ast.create_builtin(BuiltinType::builtin_operator, "*", 
        *ast.types_->get_binary_operator_type(Maps::Number, Maps::Number, Maps::Number, 2, Maps::Associativity::both));

    // TODO: subset types here
    ast.create_builtin(BuiltinType::builtin_operator, "/", 
        *ast.types_->get_binary_operator_type(Maps::Number, Maps::Number, Maps::Number, 3, Maps::Associativity::left));
}

void init_builtin_callables(Maps::AST& ast) {
    using Maps::BuiltinType;

    ast.create_builtin(BuiltinType::builtin_function, "print",
        *ast.types_->get_function_type(Maps::Void, {&Maps::String}));

    ast.create_builtin(BuiltinType::builtin_function, "print",
        *ast.types_->get_function_type(Maps::Void, {&Maps::Int}));

    ast.create_builtin(BuiltinType::builtin_function, "print",
        *ast.types_->get_function_type(Maps::Void, {&Maps::Float}));

    ast.create_builtin(BuiltinType::builtin_function, "print",
        *ast.types_->get_function_type(Maps::Void, {&Maps::Boolean}));
}