#include "mapsc/types/function_type.hh"

#define __TYPE_DEFS_CPP
#include "mapsc/types/type_defs.hh"

#include <array>

namespace Maps {

FunctionType String_to_Void{9, &Function_, &Void, {&String}, false};
FunctionType IntInt_to_Int{9, &Function_, &Int, {&Int, &Int}, false};
FunctionType FloatFloat_to_Float{9, &Function_, &Float, {&Float, &Float}, false};

std::array<const FunctionType*, 3> BUILTIN_FUNCTION_TYPES = {
    &String_to_Void,
    &IntInt_to_Int,
    &FloatFloat_to_Float
};

} // namespace Maps