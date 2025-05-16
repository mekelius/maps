#ifndef __BUILTINS_HH
#define __BUILTINS_HH

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

namespace Maps {

class Scope;

const Scope* get_builtins();

using BuiltinValue = std::variant<maps_Boolean, maps_String, maps_Int, maps_Float>;
using BuiltinBody = std::variant<External, Expression, Statement>;

class Builtin: public Callable {
public:
    Builtin(std::string_view name, const Expression&& expression);
    Builtin(std::string_view name, const Statement&& statement, const Type& type);
    Builtin(std::string_view name, External external, const Type& type);

    virtual bool is_const() const { return true; }

    BuiltinBody builtin_body_;
};

class BuiltinOperator: public Operator {
public:
    BuiltinOperator(std::string_view name, const Expression&& expression, 
        OperatorProps operator_props);

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

extern constinit BuiltinOperator unary_minus_Int;
extern constinit BuiltinOperator plus_Int;
extern constinit BuiltinOperator binary_minus_Int;
extern constinit BuiltinOperator mult_Int;;

} // namespace Maps

#endif