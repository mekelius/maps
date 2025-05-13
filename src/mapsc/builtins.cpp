#include "builtins.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/callable.hh"

using std::monostate;

namespace Maps {
namespace {

using BuiltinValue = std::variant<maps_Boolean, maps_String, maps_Int, maps_Float>;
using BuiltinBody = std::variant<External, Expression, Statement>;

class Builtin: public Callable {
public:
    Builtin(const std::string& name, const Expression&& expression)
    :Callable(name, Undefined{}, BUILTIN_SOURCE_LOCATION), 
     builtin_body_(expression) {
        body_ = &std::get<Expression>(builtin_body_);
    }

    Builtin(const std::string& name, const Statement&& statement, const Type& type)
    :Callable(name, Undefined{}, type, BUILTIN_SOURCE_LOCATION),
     builtin_body_(statement) {
        body_ = &std::get<Statement>(builtin_body_);
    }

    Builtin(const std::string& name, External external, const Type& type)
    :Callable(name, Undefined{}, type, BUILTIN_SOURCE_LOCATION), builtin_body_(external) {
        body_ = std::get<External>(builtin_body_);
    }

    BuiltinBody builtin_body_;
};


class BuiltinOperator: public Operator {
public:
    BuiltinOperator(const std::string& name, const Expression&& expression, 
        OperatorProps operator_props)
    :Operator(name, Undefined{}, operator_props, BUILTIN_SOURCE_LOCATION),
     builtin_body_(expression) {
        body_ = &std::get<Expression>(builtin_body_);
    }

    // Builtin(const std::string& name, const Statement&& statement, const Type& type)
    // :Callable(name, Undefined{}, type, BUILTIN_SOURCE_LOCATION),
    //  builtin_body_(statement) {
    //     body_ = &std::get<Statement>(builtin_body_);
    // }

    constexpr BuiltinOperator(std::string_view name, External external, const Type& type, 
        OperatorProps operator_props)
    :Operator(name, external, type, operator_props), builtin_body_(external) {}

    virtual constexpr ~BuiltinOperator() = default;

    BuiltinBody builtin_body_;
};

Builtin maps_true{"true", create_builtin_expression(true, Boolean)};
Builtin maps_false{"false", create_builtin_expression(false, Boolean)};

constexpr BuiltinOperator maps_plus_Int{"+", External{}, IntInt_to_Int,
    OperatorProps::Binary(500, Associativity::left)};
BuiltinOperator maps_minus_Int{"-", External{}, IntInt_to_Int,
    OperatorProps::Binary(510, Associativity::left)};
BuiltinOperator maps_mult_Int{"*", External{}, IntInt_to_Int,
    OperatorProps::Binary(520, Associativity::left)};
// Builtin maps_true{"/", create_builtin_expression(true, Boolean)};

Builtin maps_print{"print", External{}, String_to_Void};

static ConstScope builtins;

bool builtins_initialized = false;

} // anonymous namespace


bool init_builtins(ConstScope& scope) {
    builtins_initialized = true;

    return (
        scope.create_identifier(&maps_true     ) &&
        scope.create_identifier(&maps_false    ) &&
        scope.create_identifier(&maps_print    ) &&
        scope.create_identifier(&maps_plus_Int ) &&
        scope.create_identifier(&maps_minus_Int) &&
        scope.create_identifier(&maps_mult_Int )
    );

    // if (!ast.create_builtin_binary_operator("+", *ast.types_->get_function_type(Int, 
    //     {&Int, &Int}), 1, Associativity::left /* Associativity::both*/))
    //         return false;

    // if (!ast.create_builtin_binary_operator("-", *ast.types_->get_function_type(Int, 
    //     {&Int, &Int}), 1, Associativity::left))
    //         return false;

    // if (!ast.create_builtin_binary_operator("*", *ast.types_->get_function_type(Int, 
    //     {&Int, &Int}), 2, Associativity::left /* Associativity::both*/))
    //         return false;

    // // TODO: subset types here
    // if (!ast.create_builtin_binary_operator("/", *ast.types_->get_function_type(Int, 
    //     {&Int, &Int}), 3, Associativity::left))
    //         return false;

    // return true;

    // if (!ast.create_builtin("print",
    //     *ast.types_->get_function_type(Void, {&String})))
    //         return false;

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Void, {&Int}));

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Void, {&Float}));

    // ast.create_builtin("print",
    //     *ast.types_->get_function_type(Void, {&Boolean}));
}

const ConstScope* get_builtins() {
    if (!builtins_initialized)
        init_builtins(builtins);

    return &builtins;
}

} // namespace Maps