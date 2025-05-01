#include "builtins.hh"

#include "mapsc/ast/ast.hh"
#include "mapsc/types/type.hh"

namespace {

[[nodiscard]] bool init_builtin_operators(Maps::AST& ast) {
    if (!ast.create_builtin_binary_operator("+", *ast.types_->get_function_type(Maps::Number, 
        {&Maps::Number, &Maps::Number}), 1, Maps::Associativity::left /* Maps::Associativity::both*/))
            return false;

    if (!ast.create_builtin_binary_operator("-", *ast.types_->get_function_type(Maps::Number, 
        {&Maps::Number, &Maps::Number}), 1, Maps::Associativity::left))
            return false;

    if (!ast.create_builtin_binary_operator("*", *ast.types_->get_function_type(Maps::Number, 
        {&Maps::Number, &Maps::Number}), 2, Maps::Associativity::left /* Maps::Associativity::both*/))
            return false;

    // TODO: subset types here
    if (!ast.create_builtin_binary_operator("/", *ast.types_->get_function_type(Maps::Number, 
        {&Maps::Number, &Maps::Number}), 3, Maps::Associativity::left))
            return false;

    return true;
}

[[nodiscard]] bool init_builtin_callables(Maps::AST& ast) {
    if (!ast.create_builtin("print",
        *ast.types_->get_function_type(Maps::Void, {&Maps::String})))
            return false;

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Maps::Void, {&Maps::Int}));

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Maps::Void, {&Maps::Float}));

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Maps::Void, {&Maps::Boolean}));

    return true;
}

} // namespace

bool init_builtins(Maps::AST& ast) {
    if (!init_builtin_callables(ast))
        return false;

    if (!init_builtin_operators(ast))
        return false;

    return true;
}