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

namespace {


using BuiltinValue = std::variant<maps_Boolean, maps_String, maps_Int, maps_Float>;
using BuiltinBody = std::variant<External, Expression, Statement>;

class Builtin: public Callable {
public:
    Builtin(std::string_view name, const Expression&& expression)
    :Callable(name, Undefined{}, BUILTIN_SOURCE_LOCATION), 
     builtin_body_(expression) {
        body_ = &std::get<Expression>(builtin_body_);
    }

    Builtin(std::string_view name, const Statement&& statement, const Type& type)
    :Callable(name, Undefined{}, type, BUILTIN_SOURCE_LOCATION),
     builtin_body_(statement) {
        body_ = &std::get<Statement>(builtin_body_);
    }

    Builtin(std::string_view name, External external, const Type& type)
    :Callable(name, Undefined{}, type, BUILTIN_SOURCE_LOCATION), builtin_body_(external) {
        body_ = std::get<External>(builtin_body_);
    }

    virtual bool is_const() const { return true; }

    BuiltinBody builtin_body_;
};

class BuiltinOperator: public Operator {
public:
    BuiltinOperator(std::string_view name, const Expression&& expression, 
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

    virtual bool is_const() const { return true; }

    BuiltinBody builtin_body_;
};

Builtin maps_true{"true", create_builtin_expression(true, Boolean)};
Builtin maps_false{"false", create_builtin_expression(false, Boolean)};

constinit BuiltinOperator maps_plus_Int{"+", External{}, IntInt_to_Int,
    OperatorProps::Binary(500, Associativity::left)};
constinit BuiltinOperator maps_minus_Int{"-", External{}, IntInt_to_Int,
    OperatorProps::Binary(510, Associativity::left)};
constinit BuiltinOperator maps_mult_Int{"*", External{}, IntInt_to_Int,
    OperatorProps::Binary(520, Associativity::left)};
//constexpr Builtin maps_true{"/", create_builtin_expression(true, Boolean)};

Builtin maps_print{"print", External{}, String_to_Void};

static Scope builtins;

bool builtins_initialized = false;

} // anonymous namespace

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
        init_builtin(scope, maps_true       ) &&
        init_builtin(scope, maps_false      ) &&
        init_builtin(scope, maps_print      ) &&
        init_builtin(scope, maps_plus_Int   ) &&
        init_builtin(scope, maps_minus_Int  ) &&
        init_builtin(scope, maps_mult_Int   )
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