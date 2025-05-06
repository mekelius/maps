#ifndef __TYPES_HH
#define __TYPES_HH

#include <optional>
#include <tuple>
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
    maybe_,
};

constexpr DeferredBool db_true = DeferredBool::true_;
constexpr DeferredBool db_false = DeferredBool::false_;
constexpr DeferredBool db_maybe = DeferredBool::maybe_;

struct TypeTemplate {
    const std::string_view name;
    const DeferredBool is_native = db_false;
    const DeferredBool is_castable_to_native = db_false;
    const bool is_user_defined = false;
    const bool is_type_alias = false;
};

class Type;
struct Expression;

typedef bool(*CastFunction)(const Type*, Expression&);
typedef bool(*ConcretizeFunction)(Expression&);

class Type {
public:
    using ID = int;
    using HashableSignature = std::string;

    constexpr Type(const ID id, const TypeTemplate* type_template, const CastFunction cast_function, 
        const ConcretizeFunction concretize_function)
    :id_(id), 
     type_template_(type_template), 
     cast_function_(cast_function), 
     concretize_function_(concretize_function) {}
    
    // copy constructor
    constexpr Type(const Type& rhs):
        id_(rhs.id_), type_template_(rhs.type_template_), cast_function_(rhs.cast_function_), concretize_function_(rhs.concretize_function_) {}
    // copy assignment operator
    constexpr Type& operator=(const Type& rhs) {
        if (this == &rhs)
            return *this;

        type_template_ = rhs.type_template_;

        return *this;
    }

    constexpr bool friend operator==(const Type& lhs, const Type& rhs) {
        return lhs.id_ == rhs.id_;
    }

    // rule of 5
    // Type(Type&& rhs) = delete;
    // Type&& operator=(Type&& other) = delete;
    constexpr virtual ~Type() = default;

    virtual std::string to_string() const;

    virtual bool is_complex() const { return false; };
    virtual bool is_function() const { return false; }
    virtual unsigned int arity() const { return 0; }

    std::string_view name() const { return type_template_->name; }
    HashableSignature hashable_signature() const { return static_cast<std::string>(name()); }

    DeferredBool is_native() const { return type_template_->is_native; }
    DeferredBool is_castable_to_native() const { return type_template_->is_castable_to_native; }

    bool is_user_defined() const { return type_template_->is_user_defined; }
    bool is_type_alias() const { return type_template_->is_type_alias; }

    bool cast_to(const Type* type, Expression& expression) const;
    bool concretize(Expression& expression) const;

    const ID id_;
    const TypeTemplate* type_template_;
    const CastFunction cast_function_;

    // this function is used to force a value of this type into a concrete/native type
    const ConcretizeFunction concretize_function_;

protected:
    // cast_to above does some safety checks that shouldn't be overridden
    virtual bool cast_to_(const Type* type, Expression& expression) const;

    // concretize above does some safety checks that shouldn't be overridden
    virtual bool concretize_(Expression& expression) const;
};

// !! this struct has way too much stuff
// maybe inheritance is the way
class FunctionType: public Type {
public:
    // this is what is used as the basis for function specialization
    using HashableSignature = std::string;

    FunctionType(const ID id, const TypeTemplate* type_template, const Type* return_type, 
        const std::vector<const Type*>& arg_types, bool is_pure = false);

    const Type* return_type_;
    std::vector<const Type*> arg_types_;
    bool is_pure_ = false;

    // DO NOTE!: string representation is (currently) used as the basis for function overload specialization
    // IF YOU CREATE TYPES THAT HAVE IDENTICAL STRINGS THEIR FUNCTIONS WILL COLLIDE
    std::string to_string() const;

    // just a synonym for to_string at the moment
    // TODO: memoize this
    HashableSignature hashable_signature() const {
        return to_string();
    }

    virtual bool is_complex() const { return true; };
    virtual bool is_function() const { return true; }
    unsigned int arity() const {
        return arg_types_.size();
    }

    bool operator==(const FunctionType& other) {
        if (*dynamic_cast<const Type*>(this) != *dynamic_cast<const Type*>(&other))
            return false;

        if (this->return_type_ != other.return_type_)
            return false;

        return this->arg_types_ == other.arg_types_;
    }
};

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

    const FunctionType* get_function_type(const Type& return_type, const std::vector<const Type*>& arg_types, 
        bool pure = true);

    Type::HashableSignature make_function_signature(const Type& return_type, const std::vector<const Type*>& arg_types, 
        bool is_pure = true) const;

    const FunctionType* create_function_type(const Type::HashableSignature& signature, const Type& return_type, 
        const std::vector<const Type*>& arg_types, bool is_pure = true);

private:
    Type::ID get_id() {
        return ++next_id_;
    }

    std::unordered_map<std::string, const Type*> types_by_identifier_ = {};
    std::unordered_map<Type::HashableSignature, const Type*> types_by_structure_ = {};
    std::vector<const Type*> types_by_id_ = {};

    
    // we need two different vectors, since the builtin types need to be accessable by id as well
    std::vector<std::unique_ptr<Type>> types_ = {};
    
    Type::ID next_id_;             
    
    // std::vector<std::unique_ptr<TypeConstructor>> type_constructors_ = {};
    // std::unordered_map<std::string, const TypeConstructor*> typeconstructors_by_identifier = {};
};

} // namespace Maps
#endif