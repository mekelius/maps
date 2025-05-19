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

const CT_Scope* get_builtins();

using BuiltinValue = std::variant<maps_Boolean, maps_String, maps_Int, maps_Float>;
using BuiltinBody = std::variant<External>;

class CT_Callable: public Callable {
public:
    // CT_Callable(std::string_view name, Expression&& expression);
    // CT_Callable(std::string_view name, Statement&& statement, const Type& type);
    // CT_Callable(std::string_view name, BuiltinBody&& body, const Type& type)
    // :name_(name), 
    //  builtin_body_(body), 
    //  type_(&type), location_(BUILTIN_SOURCE_LOCATION) {}

    constexpr CT_Callable(std::string_view name, External external, const Type& type)
    :name_(name), 
     builtin_body_(external), 
     type_(&type),
     location_(BUILTIN_SOURCE_LOCATION) {}

    virtual bool is_const() const { return true; }

    virtual constexpr std::string_view name() const { return name_; }
    virtual CallableBody const_body() const;
    virtual constexpr const SourceLocation& location() const { return location_; }

    // since statements don't store types, we'll have to store them here
    // if the body is an expression, the type will just mirror it's type
    virtual const Type* get_type() const { return type_; }
    virtual std::optional<const Type*> get_declared_type() const { return std::nullopt; }
    
    virtual bool operator==(const Callable& other) const {
        if (this == &other)
            return true;

        if (const_body() == other.const_body())
            return true;

        return false;
    }

private:
    std::string_view name_;
    BuiltinBody builtin_body_;
    const Type* type_;
    SourceLocation location_;
};

class CT_Operator: public CT_Callable, public Operator {
public:
    // CT_Operator(std::string_view name, const Expression&& expression, 
    //     Operator::Properties operator_props);

    constexpr CT_Operator(std::string_view name, External external, const Type& type, 
        Operator::Properties operator_props)
    :CT_Callable(name, external, type),
     operator_props_(operator_props) {}

    constexpr bool is_operator() const { return true; }

    virtual Properties operator_props() const {
        return operator_props_;
    };

    virtual constexpr ~CT_Operator() = default;

private:
    Operator::Properties operator_props_;
};

extern constinit CT_Operator unary_minus_Int;
extern constinit CT_Operator plus_Int;
extern constinit CT_Operator binary_minus_Int;
extern constinit CT_Operator mult_Int;

} // namespace Maps

#endif