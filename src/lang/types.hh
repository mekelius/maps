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
    unknown,
};


struct FunctionType;


using TypeComplex = std::variant<std::monostate, std::unique_ptr<FunctionType>>;

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

struct FunctionType {
    Type return_type;
    std::vector<Type> arg_types;

    unsigned int arity() const {
        return arg_types.size();
    }
};
inline bool operator==(const FunctionType& lhs, const FunctionType& rhs) {
    if (lhs.return_type != rhs.return_type)
        return false;

    return lhs.arg_types == rhs.arg_types;  
}

Type create_function_type(Type return_type, const std::vector<Type>& arg_types);

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
    DeferredBool::unknown,
    DeferredBool::unknown,
};
static const Type Hole = { &Hole_ };

// a number who's type hasn't yet been determined
static TypeTemplate NumberLiteral_ {
    "NumberLiteral",
    false,
    DeferredBool::true_,
    DeferredBool::unknown,
};
static const Type NumberLiteral = { &NumberLiteral_};

} // namespace AST
#endif