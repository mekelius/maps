#ifndef __TYPES_HH
#define __TYPES_HH

#include <string>
#include <string_view>

#include "common/deferred_bool.hh"


namespace Maps {

class Type;
struct Expression;

using CastFunction = bool(const Type*, Expression&);
using ConcretizeFunction = bool(Expression&);

class Type {
public:
    constexpr virtual ~Type() = default;

    virtual bool is_complex() const { return false; }
    virtual bool is_function() const { return false; }
    virtual bool is_pure() const { return true; }
    virtual bool is_concrete() const { return false; };
    virtual uint arity() const { return 0; }
    
    virtual std::string_view name() const = 0;
    virtual bool is_voidish() const = 0;
    
    std::string to_string() const { return std::string{name()}; }
    bool is_impure() const { return !is_pure(); }
    bool is_pure_function() const { return is_pure() && is_function(); }
    bool is_impure_function() const { return !is_pure() && is_function(); }
    
    bool cast_to(const Type*, Expression&) const;

    virtual bool concretize(Expression&) const = 0;
    virtual bool operator==(const Type& other) const = 0;
    
private:
    virtual bool cast_to_(const Type*, Expression&) const = 0;
};

class RT_Type: public Type {
public:
    RT_Type(const std::string& name, CastFunction* cast_function, 
        ConcretizeFunction* concretize_function, bool is_voidish = false)
    :name_(name), 
     cast_function_(cast_function), 
     concretize_function_(concretize_function),
     is_voidish_(is_voidish) {}

    virtual std::string_view name() const { return name_; }
    virtual bool is_voidish() const { return is_voidish_; };

    virtual bool cast_to_(const Type* type, Expression& expression) const {
        return (*cast_function_)(type, expression);
    }

    virtual bool concretize(Expression& expression) const {
        return (*concretize_function_)(expression);
    }

    virtual bool operator==(const Type& other) const {
        return name() == other.name();
    }

    const std::string name_;
    CastFunction* cast_function_;
    ConcretizeFunction* concretize_function_;
    const bool is_voidish_ = false;
};

class CT_Type: public Type {
public:
    constexpr CT_Type(std::string_view name, CastFunction* const cast_function, 
        ConcretizeFunction* const concretize_function, bool is_voidish = false)
    :name_(name), 
     cast_function_(cast_function), 
     concretize_function_(concretize_function),
     is_voidish_(is_voidish) {}

    virtual std::string_view name() const { return name_; }
    virtual bool is_voidish() const { return is_voidish_; }

    virtual bool cast_to_(const Type* type, Expression& expression) const {
        return (*cast_function_)(type, expression);
    }

    virtual bool concretize(Expression& expression) const {
        return (*concretize_function_)(expression);
    }

    virtual bool operator==(const Type& other) const {
        return name() == other.name();
    }

    std::string_view name_;
    CastFunction* cast_function_;
    ConcretizeFunction* concretize_function_;
    bool is_voidish_ = false;
};

class ConcreteType: public Type {
public:
    constexpr ConcreteType(const uint concrete_type_id, std::string_view name, 
        CastFunction* const cast_function, bool is_voidish = false)
    :concrete_type_id_(concrete_type_id), 
     name_(name), 
     cast_function_(cast_function),
     is_voidish_(is_voidish) {}

    virtual bool is_concrete() const { return true; }
    virtual std::string_view name() const { return name_; }
    virtual bool is_voidish() const { return is_voidish_; }

    virtual bool cast_to_(const Type* type, Expression& expression) const {
        return (*cast_function_)(type, expression); 
    }

    virtual bool concretize(Expression& _) const { return true; }

    virtual bool operator==(const Type& other) const {
        return name() == other.name();
    }

    const uint concrete_type_id_;
    std::string_view name_;
    CastFunction* cast_function_;
    bool is_voidish_ = false;
};

} // namespace Maps
#endif