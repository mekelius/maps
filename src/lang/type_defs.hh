#ifndef __TYPE_DEFS_HH
#define __TYPE_DEFS_HH

#include "types.hh"

namespace Maps {

// ----- SIMPLE TYPES -----

static TypeTemplate Int_ {
    "Int",
    true,
    DeferredBool::true_,
    DeferredBool::true_,
};
static const Type Int = { &Int_ };

static TypeTemplate Float_ {
    "Float",
    true,
    DeferredBool::true_,
    DeferredBool::true_,
};
static const Type Float = { &Float_ };

static TypeTemplate Boolean_ {
    "Boolean",
    true,
    DeferredBool::false_,
    DeferredBool::false_,
};
static const Type Boolean = { &Boolean_ };

static TypeTemplate String_ {
    "String",
    true,
    DeferredBool::false_,
    DeferredBool::false_,
};
static const Type String = { &String_};

static TypeTemplate Void_ {
    "Void",
    true,
    DeferredBool::false_,
    DeferredBool::false_,
};
static const Type Void = { &Void_ };

static TypeTemplate Hole_ {
    "Hole",
    false,
    DeferredBool::maybe,
    DeferredBool::maybe,
};
static const Type Hole = { &Hole_ };

static TypeTemplate Number_ {
    "Number",
    false,
    DeferredBool::true_,
    DeferredBool::maybe,
};
static const Type Number = { &Number_ };

// a number who's type hasn't yet been determined
static TypeTemplate NumberLiteral_ {
    "NumberLiteral",
    false,
    DeferredBool::true_,
    DeferredBool::maybe,
};
static const Type NumberLiteral = { &NumberLiteral_};

static TypeTemplate Function_ {
    "Function",
    false,
    DeferredBool::maybe,
    DeferredBool::maybe,
};

static TypeTemplate Operator_ {
    "Operator",
    false,
    DeferredBool::maybe,
    DeferredBool::maybe,
};

static TypeTemplate Absurd_ {
    "Absurd",
    false,
    DeferredBool::false_,
    DeferredBool::false_,
};
static const Type Absurd = { &Absurd_ };

} // namespace Maps

#endif