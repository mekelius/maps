#ifndef __TYPE_DEFS_HH
#define __TYPE_DEFS_HH

#include <array>

#include "mapsc/types/type.hh"
#include "mapsc/types/casts.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/types/type_constructor.hh"


namespace Maps {

constexpr uint Void_ID      = 0;
constexpr uint Int_ID       = 1;
constexpr uint Boolean_ID   = 2;
constexpr uint Float_ID     = 3;
constexpr uint String_ID    = 4;

constexpr ConcreteType Void { Void_ID, "Void", &not_castable, true };
constexpr ConcreteType Int { Int_ID, "Int", &cast_from_Int };
constexpr ConcreteType Boolean { Boolean_ID, "Boolean", &cast_from_Boolean };
constexpr ConcreteType Float { Float_ID, "Float", &cast_from_Float };
constexpr ConcreteType String { String_ID, "String", &cast_from_String };

constexpr CT_Type Absurd{ "Absurd", &not_castable, &not_concretizable, true };
constexpr CT_Type Hole{ "Hole", &not_castable, &not_concretizable };
constexpr CT_Type Number{ "Number", &cast_from_Number, &concretize_Number };
constexpr CT_Type NumberLiteral{ "NumberLiteral", &cast_from_NumberLiteral, &concretize_NumberLiteral };
constexpr CT_Type TestingType{ "TestingType", &not_castable, &not_concretizable };

// These guys lie about their type to avoid the m-word
constexpr auto IO_Int = IO_TypeConstructor::ct_apply("IO Int", Int);
constexpr auto IO_Boolean = IO_TypeConstructor::ct_apply("IO Boolean", Boolean);
constexpr auto IO_Float = IO_TypeConstructor::ct_apply("IO Float", Float);
constexpr auto IO_String = IO_TypeConstructor::ct_apply("IO String", String);
constexpr auto IO_Void = IO_TypeConstructor::ct_apply("IO Void", Void);

constexpr std::array<const Type*, 15> BUILTIN_TYPES {
    &Int, &Boolean, &Float, &String,
    &Absurd, &Hole, &Void,
    &IO_Int, &IO_Boolean, &IO_Float, &IO_String, &IO_Void,
    &Number, &NumberLiteral,
    &TestingType
};

constexpr CTFunctionType<1> String_to_IO_Void{ "String => IO Void", &IO_Void, {&String}, false };
constexpr CTFunctionType<1> Int_to_Int{ "Int -> Int", &Int, {&Int}, true };
constexpr CTFunctionType<2> IntInt_to_Int{ "Int -> Int -> Int", &Int, {&Int, &Int}, true };
constexpr CTFunctionType<2> FloatFloat_to_Float{ "Float -> Float -> Float", &Float, {&Float, &Float}, true };
constexpr std::array<const FunctionType*, 4> BUILTIN_FUNCTION_TYPES {
    &String_to_IO_Void,
    &Int_to_Int,
    &IntInt_to_Int,
    &FloatFloat_to_Float
};

} // namespace Maps

#endif