#include "builtins.hh"

#include "ast.hh"
#include "types.hh"

void init_builtin_callables(AST::AST& ast) {
    AST::Expression* print = ast.create_expression(AST::ExpressionType::builtin_function, 
        {0, 0}, AST::Void);
    
    // print->type = AST::create_function_type(AST::Void, {AST::String});

    if (!ast.builtins_.create_identifier("print", print, {0,0})) {
        assert(false && "couldn't create builtin: print");
    }
}