#include "builtins.hh"


void init_builtin_callables(AST::AST& ast) {
    auto print_expr = ast.create_expression(ExpressionType::native_function, {0, 0}, &Void);
    auto print = create_callable("print", print_expr, &Void, { &String });
    
    if (!print)
        return false;

    (*print)->name = "print";

    return true;
}