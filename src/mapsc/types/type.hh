#ifndef __TYPES_HH
#define __TYPES_HH

#include <string>

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

// caller needs to be sure that type is an operator type
unsigned int get_precedence(const Type& type);

} // namespace Maps
#endif