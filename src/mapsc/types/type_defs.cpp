#include "mapsc/types/function_type.hh"

#define __TYPE_DEFS_CPP
#include "mapsc/types/type_defs.hh"

#include <array>

namespace Maps {

FunctionType String_to_void{9, &Function_, &Void, {&String}, false};

std::array<const FunctionType*, 1> BUILTIN_FUNCTION_TYPES = {
    &String_to_void
};

} // namespace Maps