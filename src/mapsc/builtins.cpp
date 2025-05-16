#include "builtins.hh"

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "common/maps_datatypes.h"

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/types/function_type.hh"
#include "mapsc/types/type_defs.hh"

#include "mapsc/ast/callable.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/operator.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/statement.hh"

using std::monostate;
using Maps::GlobalLogger::log_error;

namespace Maps {

class Type;

Builtin::Builtin(std::string_view name, const Expression&& expression)
:Callable(name, Undefined{}, BUILTIN_SOURCE_LOCATION), 
    builtin_body_(expression) {
    body_ = &std::get<Expression>(builtin_body_);
}

Builtin::Builtin(std::string_view name, const Statement&& statement, const Type& type)
:Callable(name, Undefined{}, type, BUILTIN_SOURCE_LOCATION),
    builtin_body_(statement) {
    body_ = &std::get<Statement>(builtin_body_);
}

Builtin::Builtin(std::string_view name, External external, const Type& type)
:Callable(name, Undefined{}, type, BUILTIN_SOURCE_LOCATION), builtin_body_(external) {
    body_ = std::get<External>(builtin_body_);
}

BuiltinOperator::BuiltinOperator(std::string_view name, const Expression&& expression, 
    OperatorProps operator_props)
:Operator(name, Undefined{}, operator_props, BUILTIN_SOURCE_LOCATION),
    builtin_body_(expression) {
    body_ = &std::get<Expression>(builtin_body_);
}

// ----- BUILTIN DEFINITIONS -----

Builtin true_{"true", create_builtin_expression(true, Boolean)};
Builtin false_{"false", create_builtin_expression(false, Boolean)};
Builtin print{"print", External{}, String_to_IO_Void};

// ----- BUILTINS SCOPE -----

static Scope builtins;
bool builtins_initialized = false;

constinit BuiltinOperator unary_minus_Int{"-", External{}, Int_to_Int,
    OperatorProps{UnaryFixity::prefix}};

constinit BuiltinOperator plus_Int{"+", External{}, IntInt_to_Int,
    OperatorProps::Binary(500, Associativity::left)};
constinit BuiltinOperator binary_minus_Int{"-", External{}, IntInt_to_Int,
    OperatorProps::Binary(510, Associativity::left)};
constinit BuiltinOperator mult_Int{"*", External{}, IntInt_to_Int,
    OperatorProps::Binary(520, Associativity::left)};

bool init_builtin(Scope& scope, Callable& callable) {
    if (!scope.create_identifier(&callable)) {
        log_error("Creating builtin " + std::string{callable.name_} + " failed");
        return false;
    }

    return true;
}

bool init_builtins(Scope& scope) {
    builtins_initialized = true;

    return (
        init_builtin(scope, true_      ) &&
        init_builtin(scope, false_     ) &&
        init_builtin(scope, print      ) &&
        init_builtin(scope, plus_Int   ) &&
        init_builtin(scope, mult_Int   )
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

const Scope* get_builtins() {
    if (!builtins_initialized && !init_builtins(builtins)) {
        log_error("Initializing builtins failed");
        assert(false && "initializing builtins failed");
    }

    return &builtins;
}

} // namespace Maps