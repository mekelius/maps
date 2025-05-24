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
constexpr uint Mut_String_ID    = 4;

constexpr auto CT_TYPES_START_LINE = __LINE__;
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
constexpr auto IO_Int = IO_TypeConstructor::ct_apply("IO Int", Int);
constexpr auto IO_Boolean = IO_TypeConstructor::ct_apply("IO Boolean", Boolean);
constexpr auto IO_Float = IO_TypeConstructor::ct_apply("IO Float", Float);
constexpr auto IO_String = IO_TypeConstructor::ct_apply("IO String", String);
constexpr auto IO_Void = IO_TypeConstructor::ct_apply("IO Void", Void);
constexpr ConcreteType Mut_String = { Mut_String_ID, "String'", &cast_from_Mut_String };
constexpr auto CT_TYPES_COUNT = __LINE__ - CT_TYPES_START_LINE - 1;

constexpr std::array<const Type*, CT_TYPES_COUNT> BUILTIN_TYPES {
    &Void, &Int, &Boolean, &Float, &String,
    &Absurd, &Hole, &Number, &NumberLiteral, &TestingType,
    &IO_Int, &IO_Boolean, &IO_Float, &IO_String, &IO_Void,

    &Mut_String
};

constexpr auto CT_FUNCTION_TYPES_START_LINE = __LINE__;
constexpr CTFunctionType<1> String_to_IO_Void{ "String => IO Void", &IO_Void, {&String}, false };
constexpr CTFunctionType<1> Int_to_Int{ "Int -> Int", &Int, {&Int}, true };
constexpr CTFunctionType<2> IntInt_to_Int{ "Int -> Int -> Int", &Int, {&Int, &Int}, true };
constexpr CTFunctionType<2> FloatFloat_to_Float{ "Float -> Float -> Float", &Float, {&Float, &Float}, true };
constexpr CTFunctionType<1> Boolean_to_String{ "Boolean -> String", &String, {&Boolean}, true};
constexpr CTFunctionType<1> Int_to_String{ "Int -> String", &String, {&Int}, true};
constexpr CTFunctionType<1> Float_to_String{ "Float -> String", &String, {&Float}, true};
constexpr CTFunctionType<1> Int_to_Float{ "Int -> Float", &Float, {&Int}, true};
constexpr auto CT_FUNCTION_TYPES_COUNT = __LINE__ - CT_FUNCTION_TYPES_START_LINE - 1;

constexpr std::array<const FunctionType*, CT_FUNCTION_TYPES_COUNT> BUILTIN_FUNCTION_TYPES {
    &String_to_IO_Void,
    &Int_to_Int,
    &IntInt_to_Int,
    &FloatFloat_to_Float,
    &Boolean_to_String,
    &Int_to_String,
    &Float_to_String,
    &Int_to_Float
};

} // namespace Maps

#endif