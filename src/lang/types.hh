#ifndef __TYPES_HH
#define __TYPES_HH

#include <optional>
#include <variant>
#include <vector>
#include <string>
#include <string_view>
#include <memory>

namespace AST {

// class for booleans we may or may not know yet
enum class DeferredBool {
    true_,
    false_,
    maybe,
};

// is this even a word
enum class Fixity {
    prefix,
    infix,
    postfix,
};

enum class Associativity {
    left,
    right,
    both,
    none,
};

struct FunctionTypeComplex;

using TypeComplex = std::variant<std::monostate, std::unique_ptr<FunctionTypeComplex>>;

struct TypeTemplate {
    const std::string_view name;
    const bool is_native = false;
    const DeferredBool is_numeric = DeferredBool::false_;
    const DeferredBool is_integral = DeferredBool::false_;
    const bool is_user_defined = false;
    const bool is_type_alias = false;
};

struct Type {
    TypeTemplate* type_template;
    TypeComplex complex = std::monostate{};

    Type(TypeTemplate* type_template);
    Type(const Type& rhs);
    Type& operator=(const Type& other);

    // rule of 5
    // Type(Type&& rhs) = delete;
    // Type&& operator=(Type&& other) = delete;

    ~Type() = default;

    bool is_complex() const;   
    unsigned int arity() const;
    unsigned int precedence() const;
    FunctionTypeComplex* function_type() const {
        return std::get<std::unique_ptr<FunctionTypeComplex>>(complex).get();
    }

    std::string_view name() const { return type_template->name; }
    bool is_native() const { return type_template->is_native; }
    DeferredBool is_numeric() const { return type_template->is_numeric; }
    DeferredBool is_integral() const { return type_template->is_integral; }
    bool is_user_defined() const { return type_template->is_user_defined; }
    bool is_type_alias() const { return type_template->is_type_alias; }
};

// currently simple types are only compared based on their name 
// TODO: need to prevent user created types from colliding
bool operator==(const Type& lhs, const Type& rhs);
inline bool operator!=(const Type& lhs, const Type& rhs) {
    return !(lhs == rhs);
}

// !! this struct has way too much stuff
// maybe inheritance is the way
struct FunctionTypeComplex {
    Type return_type;
    std::vector<Type> arg_types;
    bool is_operator = false;
    Fixity fixity = Fixity::prefix;
    unsigned int precedence = 999;
    bool is_arithmetic_operator = false;
    Associativity associativity = Associativity::none;

    unsigned int arity() const {
        return arg_types.size();
    }
};
inline bool operator==(const FunctionTypeComplex& lhs, const FunctionTypeComplex& rhs) {
    if (lhs.is_operator != rhs.is_operator)
        return false;

    if (lhs.return_type != rhs.return_type)
        return false;

    return lhs.arg_types == rhs.arg_types;  
}

Type create_function_type(const Type& return_type, const std::vector<Type>& arg_types);
Type create_binary_operator_type(const Type& return_type, const Type& lhs, const Type& rhs, 
    unsigned int precedence, bool is_arithmetic = false, 
    Associativity associativity = Associativity::none);

// caller needs to be sure that type is an operator type
unsigned int get_precedence(const Type& type);

// ----- SIMPLE TYPES -----

static TypeTemplate Int_ {
    "Int",
    true,
    DeferredBool::true_,
    DeferredBool::true_,
};
static const Type Int = { &Int_ };

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

} // namespace AST
#endif