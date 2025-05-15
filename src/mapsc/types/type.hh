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
    virtual ~Type() = default;

    virtual bool is_complex() const { return false; }
    virtual bool is_function() const { return false; }
    virtual bool is_pure() const { return true; }
    virtual bool is_concrete() const { return false; };
    virtual uint arity() const { return 0; }
    
    virtual std::string_view name() const = 0;

    std::string to_string() const {
        return std::string{name()};
    }


    virtual bool cast_to(const Type*, Expression&) const = 0;
    virtual bool concretize(Expression&) const = 0;
    virtual bool operator==(const Type& other) const = 0;
};

class RT_Type: public Type {
public:
    RT_Type(const std::string& name, CastFunction* cast_function, ConcretizeFunction* concretize_function)
    :name_(name), cast_function_(cast_function), concretize_function_(concretize_function) {}

    virtual std::string_view name() const {
        return name_;
    }

    virtual bool cast_to(const Type* type, Expression& expression) const {
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
};

class CT_Type: public Type {
public:
    constexpr CT_Type(std::string_view name, CastFunction* const cast_function, 
        ConcretizeFunction* const concretize_function)
    :name_(name), cast_function_(cast_function), concretize_function_(concretize_function) {}

    virtual std::string_view name() const {
        return name_;
    }

    virtual bool cast_to(const Type* type, Expression& expression) const {
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
};

class ConcreteType: public Type {
public:
    constexpr ConcreteType(const uint concrete_type_id, std::string_view name, 
        CastFunction* const cast_function)
    :concrete_type_id_(concrete_type_id), name_(name), cast_function_(cast_function) {}

    virtual bool is_concrete() const { return true; }

    virtual std::string_view name() const {
        return name_;
    }

    virtual bool cast_to(const Type* type, Expression& expression) const {
        return (*cast_function_)(type, expression);
    }

    virtual bool concretize(Expression&) const {
        return true;
    }

    virtual bool operator==(const Type& other) const {
        return name() == other.name();
    }

    const uint concrete_type_id_;
    std::string_view name_;
    CastFunction* cast_function_;
};

} // namespace Maps
#endif