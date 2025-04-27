#ifndef __TYPE_DEFS_HH
#define __TYPE_DEFS_HH

#include <array>

#include "type.hh"

namespace Maps {

constexpr TypeTemplate Absurd_ {
    "Absurd",
    false,
    DeferredBool::false_,
    DeferredBool::false_,
};
constexpr Type Absurd{ 0, &Absurd_ };

static TypeTemplate Hole_ {
    "Hole",
    false,
    DeferredBool::maybe,
    DeferredBool::maybe,
};
static const Type Hole = { 1, &Hole_ };

static TypeTemplate Void_ {
    "Void",
    true,
    DeferredBool::false_,
    DeferredBool::false_,
};
static const Type Void = { 2, &Void_ };


static TypeTemplate Boolean_ {
    "Boolean",
    true,
    DeferredBool::false_,
    DeferredBool::false_,
};
static const Type Boolean = { 3, &Boolean_ };

static TypeTemplate Int_ {
    "Int",
    true,
    DeferredBool::true_,
    DeferredBool::true_,
};
static const Type Int{ 4, &Int_ };

static TypeTemplate Float_ {
    "Float",
    true,
    DeferredBool::true_,
    DeferredBool::true_,
};
static const Type Float{ 5, &Float_ };

static TypeTemplate String_ {
    "String",
    true,
    DeferredBool::false_,
    DeferredBool::false_,
};
static const Type String = { 6, &String_};

constexpr TypeTemplate Number_ {
    "Number",
    false,
    DeferredBool::true_,
    DeferredBool::maybe,
};
constexpr Type Number{ 7, &Number_ };

// a number who's type hasn't yet been determined
static TypeTemplate NumberLiteral_ {
    "NumberLiteral",
    false,
    DeferredBool::true_,
    DeferredBool::maybe,
};
static const Type NumberLiteral = { 8, &NumberLiteral_};

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

static TypeTemplate Function_ {
    "Function",
    false,
    DeferredBool::maybe,
    DeferredBool::maybe,
};


namespace TypeConstructors {

class PureFunctionConstructor: public TypeConstructor {
public:
    PureFunctionConstructor();
    const Type* make_type(const std::vector<TypeArg>& args, std::string* name = nullptr);
};
static const PureFunctionConstructor PureFunction{};

class ImpureFunctionConstructor: public TypeConstructor {
public:
    ImpureFunctionConstructor();
    const Type* make_type(const std::vector<TypeArg>& args, std::string* name = nullptr);
};
static const PureFunctionConstructor ImpureFunction{};

// static constexpr TypeConstructor optional;
// static constexpr TypeConstructor tuple;
// static constexpr TypeConstructor union_tc;
// static constexpr TypeConstructor sequence;

static const std::array<const TypeConstructor*, 2> BUILTINS = {
    &PureFunction, 
    &ImpureFunction, 
};

} // namespace TypeConstructors


} // namespace Maps

#endif