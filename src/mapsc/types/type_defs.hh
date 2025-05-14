#ifndef __TYPE_DEFS_HH
#define __TYPE_DEFS_HH

#include <array>

#include "mapsc/types/type.hh"
#include "mapsc/types/casts.hh"
#include "mapsc/types/function_type.hh"

namespace Maps {

constexpr TypeTemplate Absurd_ {
    db_false,
    db_false,
};
constexpr Type Absurd{ "Absurd", &Absurd_, not_castable, not_concretizable };

constexpr Type TestingType{ "TestingType", &Absurd_, 
    not_castable, not_concretizable };

constexpr TypeTemplate Hole_ {
    db_false,
    db_false,
};
constexpr Type Hole{ "Hole", &Hole_, not_castable, not_concretizable };

constexpr TypeTemplate Void_ {
    db_true,
    db_true,
};
constexpr Type Void{ "Void", &Void_, not_castable, is_concrete };

constexpr TypeTemplate Boolean_ {
    db_true,
    db_true,
};
constexpr uint Boolean_ID = 0;
constexpr ConcreteType Boolean{ Boolean_ID, "Boolean", &Boolean_, cast_from_Boolean, is_concrete };

constexpr TypeTemplate Int_ {
    db_true,
    db_true,
};
constexpr uint Int_ID = 1;
constexpr ConcreteType Int{ Int_ID, "Int", &Int_, cast_from_Int, is_concrete };

constexpr TypeTemplate Float_ {
    db_true,
    db_true,
};
constexpr uint Float_ID = 2;
constexpr ConcreteType Float{ Float_ID, "Float", &Float_, cast_from_Float, is_concrete };

constexpr TypeTemplate String_ {
    db_true,
    db_true,
};
constexpr uint String_ID = 3;
constexpr ConcreteType String{ String_ID, "String", &String_, cast_from_String, is_concrete };

constexpr TypeTemplate Number_ {
    db_false,
    db_true,
};
constexpr Type Number{ "Number", &Number_, cast_from_Number, concretize_Number };

// a number who's type hasn't yet been determined
constexpr TypeTemplate NumberLiteral_ {
    db_false,
    db_true,
};
constexpr Type NumberLiteral{ 
    "NumberLiteral", &NumberLiteral_, cast_from_NumberLiteral, concretize_NumberLiteral};

constexpr TypeTemplate UnaryWrapperType_ {
    db_false,
    db_true
};

constexpr std::array<const Type*, 10> BUILTIN_TYPES = {
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

constexpr CTFunctionType<1> String_to_Void{&Void, {&String}, false};
constexpr CTFunctionType<2> IntInt_to_Int{&Int, {&Int, &Int}, true};
constexpr CTFunctionType<2> FloatFloat_to_Float{&Float, {&Float, &Float}, true};
constexpr CTFunctionType<1> Void_to_Void{&Void, {&Void}, false};
constexpr std::array<const FunctionType*, 4> BUILTIN_FUNCTION_TYPES = {
    &String_to_Void,
    &IntInt_to_Int,
    &FloatFloat_to_Float,
    &Void_to_Void
};

} // namespace Maps

#endif