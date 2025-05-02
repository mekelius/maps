#ifndef __TYPE_DEFS_HH
#define __TYPE_DEFS_HH

#include <array>

#include "casts.hh"
#include "type.hh"

namespace Maps {

constexpr TypeTemplate Absurd_ {
    "Absurd",
    false,
    DeferredBool::false_,
    DeferredBool::false_,
};
constexpr Type::ID Absurd_ID = 0;
constexpr Type Absurd{ Absurd_ID, &Absurd_, not_castable };

constexpr TypeTemplate Hole_ {
    "Hole",
    false,
    DeferredBool::maybe,
    DeferredBool::maybe,
};
constexpr Type::ID Hole_ID = 1;
constexpr Type Hole = { Hole_ID, &Hole_, not_castable };

constexpr TypeTemplate Void_ {
    "Void",
    true,
    DeferredBool::false_,
    DeferredBool::false_,
};
constexpr Type::ID Void_ID = 2;
constexpr Type Void = { Void_ID, &Void_, not_castable };

constexpr TypeTemplate Boolean_ {
    "Boolean",
    true,
    DeferredBool::false_,
    DeferredBool::false_,
};
constexpr Type::ID Boolean_ID = 3;
constexpr Type Boolean = { Boolean_ID, &Boolean_, cast_from_Boolean };

constexpr TypeTemplate Int_ {
    "Int",
    true,
    DeferredBool::true_,
    DeferredBool::true_,
};
constexpr Type::ID Int_ID = 4;
constexpr Type Int{ Int_ID, &Int_, cast_from_Int };

constexpr TypeTemplate Float_ {
    "Float",
    true,
    DeferredBool::true_,
    DeferredBool::true_,
};
constexpr Type::ID Float_ID = 5;
constexpr Type Float{ Float_ID, &Float_, cast_from_Float };

constexpr TypeTemplate String_ {
    "String",
    true,
    DeferredBool::false_,
    DeferredBool::false_,
};
constexpr Type::ID String_ID = 6;
constexpr Type String = { String_ID, &String_, cast_from_String};

constexpr TypeTemplate Number_ {
    "Number",
    false,
    DeferredBool::true_,
    DeferredBool::maybe,
};
constexpr Type::ID Number_ID = 7;
constexpr Type Number{ Number_ID, &Number_, cast_from_Number };

// a number who's type hasn't yet been determined
constexpr TypeTemplate NumberLiteral_ {
    "NumberLiteral",
    false,
    DeferredBool::true_,
    DeferredBool::maybe,
};
constexpr Type::ID NumberLiteral_ID = 8;
constexpr Type NumberLiteral = { NumberLiteral_ID, &NumberLiteral_, cast_from_NumberLiteral};

static const std::array<const Type*, 9> BUILTIN_TYPES = {
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
    false,
    DeferredBool::maybe,
    DeferredBool::maybe,
};

} // namespace Maps

#endif