#include "mapsc/types/function_type.hh"

#define __TYPE_DEFS_CPP
#include "mapsc/types/type_defs.hh"

#include <array>

namespace Maps {

CTFunctionType<1> String_to_Void{10, &Void, {&String}, false};
CTFunctionType<2> IntInt_to_Int{11, &Int, {&Int, &Int}, true};
CTFunctionType<2> FloatFloat_to_Float{12, &Float, {&Float, &Float}, true};

std::array<const FunctionType*, 3> BUILTIN_FUNCTION_TYPES = {
    &String_to_Void,
    &IntInt_to_Int,
    &FloatFloat_to_Float
};

} // namespace Maps