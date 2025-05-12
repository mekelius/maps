#include "builtins.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/types/type.hh"

namespace Maps {

static Scope builtins;

const Scope* get_builtins() {
    return &builtins;
}

// Callable* create_builtin(const std::string& name, const Type& type) {
//     builtins_.push_back(std::make_unique<Builtin>(name, &type));
//     Builtin* builtin = builtins_.back().get();

//     assert(!builtins_scope_->identifier_exists(name)
//         && "tried to redefine an existing builtin");
    
//     return *builtins_scope_->create_callable(name, builtin);
// }

// // Callable* create_builtin_binary_operator(const std::string& name, const Type& type, 
// //     Precedence precedence, Associativity Associativity) {
    
// //     assert(type.arity() >= 2 && "AST::create_builtin_binary_operator called with arity < 2");

// //     Callable* callable = create_builtin(name, type);
// //     callable->operator_props = create_operator({
// //         UnaryFixity::none, BinaryFixity::infix, precedence, Associativity});

// //     return callable;
// // }

// // Callable* create_builtin_unary_operator(const std::string& name, const Type& type, UnaryFixity fixity) {
    
// //     assert(type.arity() >= 1 && "AST::create_builtin_unary_operator called with arity < 1");

// //     Callable* callable = create_builtin(name, type);
// //     callable->operator_props = create_operator({fixity, BinaryFixity::none});

// //     return callable;
// // }
    
// [[nodiscard]] bool init_builtin_operators(AST_Store& ast) {
//     if (!ast.create_builtin_value("true", true, Boolean))
//         return false;

//     if (!ast.create_builtin_value("false", false, Boolean))
//         return false;

//     if (!ast.create_builtin_binary_operator("+", *ast.types_->get_function_type(Int, 
//         {&Int, &Int}), 1, Associativity::left /* Associativity::both*/))
//             return false;

//     if (!ast.create_builtin_binary_operator("-", *ast.types_->get_function_type(Int, 
//         {&Int, &Int}), 1, Associativity::left))
//             return false;

//     if (!ast.create_builtin_binary_operator("*", *ast.types_->get_function_type(Int, 
//         {&Int, &Int}), 2, Associativity::left /* Associativity::both*/))
//             return false;

//     // TODO: subset types here
//     if (!ast.create_builtin_binary_operator("/", *ast.types_->get_function_type(Int, 
//         {&Int, &Int}), 3, Associativity::left))
//             return false;

//     return true;
// }

// [[nodiscard]] bool init_builtin_callables() {
//     if (!ast.create_builtin("print",
//         *ast.types_->get_function_type(Void, {&String})))
//             return false;

//     // ast.create_builtin("print",
//     //     *ast.types_->get_function_type(Void, {&Int}));

//     // ast.create_builtin("print",
//     //     *ast.types_->get_function_type(Void, {&Float}));

//     // ast.create_builtin("print",
//     //     *ast.types_->get_function_type(Void, {&Boolean}));

//     return true;
// }

} // namespace Maps