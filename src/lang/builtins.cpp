#include "builtins.hh"

#include "ast.hh"
#include "types.hh"

void init_builtins(AST::AST& ast) {
    init_builtin_callables(ast);
    init_builtin_operators(ast);
}

void init_builtin_operators(AST::AST& ast) {
    using AST::BuiltinType;

    ast.create_builtin(BuiltinType::builtin_operator, "+",
        AST::create_binary_operator_type(AST::Number, AST::Number, AST::Number, 1, AST::Associativity::both));

    ast.create_builtin(BuiltinType::builtin_operator, "-", 
        AST::create_binary_operator_type(AST::Number, AST::Number, AST::Number, 1, AST::Associativity::left));

    ast.create_builtin(BuiltinType::builtin_operator, "*", 
        AST::create_binary_operator_type(AST::Number, AST::Number, AST::Number, 2, AST::Associativity::both));

    // TODO: subset types here
    ast.create_builtin(BuiltinType::builtin_operator, "/", 
        AST::create_binary_operator_type(AST::Number, AST::Number, AST::Number, 3, AST::Associativity::left));
}

void init_builtin_callables(AST::AST& ast) {
    using AST::BuiltinType;

    ast.create_builtin(BuiltinType::builtin_function, "print",
        AST::create_function_type(AST::Void, {AST::String}));

    ast.create_builtin(BuiltinType::builtin_function, "print",
        AST::create_function_type(AST::Void, {AST::Int}));

    ast.create_builtin(BuiltinType::builtin_function, "print",
        AST::create_function_type(AST::Void, {AST::Float}));

    ast.create_builtin(BuiltinType::builtin_function, "print",
        AST::create_function_type(AST::Void, {AST::Boolean}));
}