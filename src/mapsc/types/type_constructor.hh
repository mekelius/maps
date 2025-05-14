#ifndef __TYPE_CONSTRUCTOR_HH
#define __TYPE_CONSTRUCTOR_HH

#include <string_view>
#include <functional>
#include <span>

#include "mapsc/types/type.hh"

namespace Maps {

class UnaryTypeConstructor {
public:
    constexpr UnaryTypeConstructor(std::string_view name)
     :name_(name) {}

    std::string_view name_;
    
    constexpr Type apply(const Type& type_arg) {
        return Type{};
    }
};

} // namespace Maps

#endif
