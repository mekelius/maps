#ifndef __TYPE_DEFS_HH
#define __TYPE_DEFS_HH

#include <array>

#include "mapsc/types/type.hh"
#include "mapsc/types/casts.hh"
#include "mapsc/types/function_type.hh"

namespace Maps {

constexpr CT_Type Absurd{ "Absurd", &not_castable, &not_concretizable };

constexpr CT_Type TestingType{ "TestingType", &not_castable, &not_concretizable };

constexpr CT_Type Hole{ "Hole", &not_castable, &not_concretizable };

constexpr CT_Type Void{ "Void", &not_castable, &is_concrete };

constexpr uint Boolean_ID = 0;
constexpr ConcreteType Boolean{ Boolean_ID, "Boolean", &cast_from_Boolean };

constexpr uint Int_ID = 1;
constexpr ConcreteType Int{ Int_ID, "Int", &cast_from_Int };

constexpr uint Float_ID = 2;
constexpr ConcreteType Float{ Float_ID, "Float", &cast_from_Float };

constexpr uint String_ID = 3;
constexpr ConcreteType String{ String_ID, "String", &cast_from_String };

constexpr CT_Type Number{ "Number", &cast_from_Number, &concretize_Number };

constexpr CT_Type NumberLiteral{ "NumberLiteral", &cast_from_NumberLiteral, &concretize_NumberLiteral };

constexpr std::array<const Type*, 10> BUILTIN_TYPES {
    &Absurd,
    &TestingType,
    &Hole,
    &Void,
    &Boolean,
    &Int,
    &Float,
    &String,
    &Number,
    &NumberLiteral
};

constexpr CTFunctionType<1> String_to_Void{ "String => Void", &Void, {&String}, false };
constexpr CTFunctionType<1> Void_to_Void{ "Void => Void", &Void, {&Void}, false };
constexpr CTFunctionType<2> IntInt_to_Int{ "Int -> Int -> Int", &Int, {&Int, &Int}, true };
constexpr CTFunctionType<2> FloatFloat_to_Float{ "Float -> Float -> Float", &Float, {&Float, &Float}, true };
constexpr std::array<const FunctionType*, 4> BUILTIN_FUNCTION_TYPES {
    &String_to_Void,
    &IntInt_to_Int,
    &FloatFloat_to_Float,
    &Void_to_Void
};

} // namespace Maps

#endif