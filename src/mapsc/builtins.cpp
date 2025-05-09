#include "builtins.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/types/type.hh"

namespace Maps {

namespace {

[[nodiscard]] bool init_builtin_operators(AST_Store& ast) {
    if (!ast.create_builtin_binary_operator("+", *ast.types_->get_function_type(Int, 
        {&Int, &Int}), 1, Associativity::left /* Associativity::both*/))
            return false;

    if (!ast.create_builtin_binary_operator("-", *ast.types_->get_function_type(Int, 
        {&Int, &Int}), 1, Associativity::left))
            return false;

    if (!ast.create_builtin_binary_operator("*", *ast.types_->get_function_type(Int, 
        {&Int, &Int}), 2, Associativity::left /* Associativity::both*/))
            return false;

    // TODO: subset types here
    if (!ast.create_builtin_binary_operator("/", *ast.types_->get_function_type(Int, 
        {&Int, &Int}), 3, Associativity::left))
            return false;

    return true;
}

[[nodiscard]] bool init_builtin_callables(AST_Store& ast) {
    if (!ast.create_builtin("print",
        *ast.types_->get_function_type(Void, {&String})))
            return false;

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Void, {&Int}));

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Void, {&Float}));

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Void, {&Boolean}));

    return true;
}

} // namespace

bool init_builtins(AST_Store& ast) {
    if (!init_builtin_callables(ast))
        return false;

    if (!init_builtin_operators(ast))
        return false;

    return true;
}

} // namespace Maps