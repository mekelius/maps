#ifndef __TYPE_DEFS_HH
#define __TYPE_DEFS_HH

#include <array>

#include "mapsc/types/type.hh"
#include "mapsc/types/casts.hh"
#include "mapsc/types/function_type.hh"

namespace Maps {

constexpr TypeTemplate Absurd_ {
    "Absurd",
    db_false,
    db_false,
};
constexpr Type::ID Absurd_ID = 0;
constexpr Type Absurd{ Absurd_ID, &Absurd_, not_castable, not_concretizable };

constexpr TypeTemplate Hole_ {
    "Hole",
    db_false,
    db_false,
};
constexpr Type::ID Hole_ID = 1;
constexpr Type Hole{ Hole_ID, &Hole_, not_castable, not_concretizable };

constexpr TypeTemplate Void_ {
    "Void",
    db_true,
    db_true,
};
constexpr Type::ID Void_ID = 2;
constexpr Type Void{ Void_ID, &Void_, not_castable, is_concrete };

constexpr TypeTemplate Boolean_ {
    "Boolean",
    db_true,
    db_true,
};
constexpr Type::ID Boolean_ID = 3;
constexpr Type Boolean = { Boolean_ID, &Boolean_, cast_from_Boolean, is_concrete };

constexpr TypeTemplate Int_ {
    "Int",
    db_true,
    db_true,
};
constexpr Type::ID Int_ID = 4;
constexpr Type Int{ Int_ID, &Int_, cast_from_Int, is_concrete };

constexpr TypeTemplate Float_ {
    "Float",
    db_true,
    db_true,
};
constexpr Type::ID Float_ID = 5;
constexpr Type Float{ Float_ID, &Float_, cast_from_Float, is_concrete };

constexpr TypeTemplate String_ {
    "String",
    db_true,
    db_true,
};
constexpr Type::ID String_ID = 6;
constexpr Type String{ String_ID, &String_, cast_from_String, is_concrete };

constexpr TypeTemplate Number_ {
    "Number",
    db_false,
    db_true,
};
constexpr Type::ID Number_ID = 7;
constexpr Type Number{ Number_ID, &Number_, cast_from_Number, concretize_Number };

// a number who's type hasn't yet been determined
constexpr TypeTemplate NumberLiteral_ {
    "NumberLiteral",
    db_false,
    db_true,
};
constexpr Type::ID NumberLiteral_ID = 8;
constexpr Type NumberLiteral{ 
    NumberLiteral_ID, &NumberLiteral_, cast_from_NumberLiteral, concretize_NumberLiteral};

constexpr std::array<const Type*, 9> BUILTIN_TYPES = {
    &Absurd,
    &Hole,
    &Void,
    &Boolean,
    &Int,
    &Float,
    &String,
    &Number,
    &NumberLiteral
};

constexpr TypeTemplate Function_ {
    "Function",
    db_maybe,
    db_maybe,
};

#ifndef __TYPE_DEFS_CPP
extern FunctionType String_to_void;
extern std::array<const FunctionType*, 1> BUILTIN_FUNCTION_TYPES;
#endif

} // namespace Maps

#endif