#ifndef __TYPES_HH
#define __TYPES_HH

#include <optional>
#include <variant>
#include <vector>
#include <string>
#include <string_view>
#include <memory>

namespace Maps {

constexpr unsigned int MAX_OPERATOR_PRECEDENCE = 1000;
constexpr unsigned int MIN_OPERATOR_PRECEDENCE = 0;

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

    std::string to_string() const;

    bool is_complex() const;
    bool is_operator() const;
    bool is_function() const;
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
    // this is what is used as the basis for function specialization
    using HashableSignature = std::string;

    Type return_type;
    std::vector<Type> arg_types;
    bool is_operator = false;
    Fixity fixity = Fixity::prefix;
    unsigned int precedence = 999;
    Associativity associativity = Associativity::none;

    // DO NOTE!: string representation is (currently) used as the basis for function overload specialization
    // IF YOU CREATE TYPES THAT HAVE IDENTICAL STRINGS THEIR FUNCTIONS WILL COLLIDE
    std::string to_string() const;

    // just a synonym for to_string at the moment
    // TODO: memoize this
    HashableSignature hashable_signature() const {
        return to_string();
    }

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
    unsigned int precedence, Associativity associativity = Associativity::none);
Type create_unary_operator_type(const Type& return_type, const Type& arg_type, Fixity fixity);

// caller needs to be sure that type is an operator type
unsigned int get_precedence(const Type& type);


// class for holding the shared type information such as traits
class TypeRegistry {
    public:
    
    private:
};

} // namespace Maps
#endif