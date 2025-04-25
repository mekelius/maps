#ifndef __TYPES_HH
#define __TYPES_HH

#include <optional>
#include <variant>
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>

namespace Maps {

// class for booleans we may or may not know yet
enum class DeferredBool {
    true_,
    false_,
    maybe,
};

class FunctionTypeComplex;

using TypeComplex = std::variant<std::monostate, std::unique_ptr<FunctionTypeComplex>>;

struct TypeTemplate {
    const std::string_view name;
    const bool is_native = false;
    const DeferredBool is_numeric = DeferredBool::false_;
    const DeferredBool is_integral = DeferredBool::false_;
    const bool is_user_defined = false;
    const bool is_type_alias = false;
};

class Type {    
public:
    using HashableSignature = std::string;
    using ID = unsigned int;

    TypeTemplate* type_template;
    TypeComplex complex = std::monostate{};

    Type(ID id, TypeTemplate* type_template);

    // copy constructor
    Type(const Type& rhs);
    Type& operator=(const Type& other);

    // rule of 5
    // Type(Type&& rhs) = delete;
    // Type&& operator=(Type&& other) = delete;

    ~Type() = default;

    std::string to_string() const;

    bool is_complex() const;
    bool is_function() const;
    unsigned int arity() const;

    FunctionTypeComplex* function_type() const {
        return std::get<std::unique_ptr<FunctionTypeComplex>>(complex).get();
    }

    std::string_view name() const { return type_template->name; }
    bool is_native() const { return type_template->is_native; }
    DeferredBool is_numeric() const { return type_template->is_numeric; }
    DeferredBool is_integral() const { return type_template->is_integral; }
    bool is_user_defined() const { return type_template->is_user_defined; }
    bool is_type_alias() const { return type_template->is_type_alias; }

    ID id;
};

// currently simple types are only compared based on their name 
// TODO: need to prevent user created types from colliding
bool operator==(const Type& lhs, const Type& rhs);
inline bool operator!=(const Type& lhs, const Type& rhs) {
    return !(lhs == rhs);
}

// !! this struct has way too much stuff
// maybe inheritance is the way
class FunctionTypeComplex {
public:
    // this is what is used as the basis for function specialization
    using HashableSignature = std::string;

    const Type* return_type;
    std::vector<const Type*> arg_types;
    bool is_pure = false;

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
    if (lhs.return_type != rhs.return_type)
        return false;

    return lhs.arg_types == rhs.arg_types;  
}

// caller needs to be sure that type is an operator type
unsigned int get_precedence(const Type& type);


// class for holding the shared type information such as traits
// See identifying_types text file for better description
class TypeRegistry {
public:
    TypeRegistry();

    std::optional<const Type*> create_type(const std::string& identifier, const TypeTemplate& template_);
    
    std::optional<const Type*> get(const std::string& identifier);
    const Type* get_unsafe(const std::string& identifier);

    const Type* get_function_type(const Type& return_type, const std::vector<const Type*>& arg_types, 
        bool pure = true);

    const Type* create_opaque_alias(std::string name, const Type* type);
    const Type* create_transparent_alias(std::string name, const Type* type);

    Type::HashableSignature make_function_signature(const Type& return_type, const std::vector<const Type*>& arg_types, 
        bool is_pure = true) const;

    const Type* create_function_type(const Type::HashableSignature& signature, const Type& return_type, 
        const std::vector<const Type*>& arg_types, bool is_pure = true);

private:
    Type::ID get_id() {
        return ++next_id_;
    }

    std::unordered_map<std::string, const Type*> types_by_identifier_;
    std::unordered_map<Type::HashableSignature, const Type*> types_by_structure_;
    std::vector<const Type*> types_by_id_;

    // we need two different vectors, since the builtin types need to be accessable by id as well
    std::vector<std::unique_ptr<Type>> types_;

    Type::ID next_id_;             
};

} // namespace Maps
#endif