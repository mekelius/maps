#ifndef __BUILTIN_HH
#define __BUILTIN_HH

#include <string_view>

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/ast/definition_body.hh"

namespace Maps {

class Type;

struct BuiltinValue {
    std::string_view name_;
    std::variant<maps_Boolean, maps_String, maps_Int, maps_Float> value_;
    const Type* type_;

    std::string_view log_representation() const { return name_; }
};

template<size_t size_p>
using BuiltinValueScope = TBuiltinScope<const BuiltinValue*, size_p>;

} // namespace Maps

#endif