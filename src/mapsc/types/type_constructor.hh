#ifndef __TYPE_CONSTRUCTOR_HH
#define __TYPE_CONSTRUCTOR_HH

#include <string_view>
#include <functional>
#include <span>
#include <ranges>

#include "mapsc/types/type.hh"

namespace Maps {

class IO_WrappedType: public Type {
public:
    IO_WrappedType(const std::string& name, const Type& wrapped_type)
    :name_(name),
     wrapped_type_(&wrapped_type) {}

    virtual bool is_pure() const { return false; }
    virtual bool is_concrete() const { return wrapped_type_->is_concrete(); }
    virtual bool is_complex() const { return true; }
    virtual std::string_view name() const { return name_; }
    virtual bool is_voidish() const { return wrapped_type_->is_voidish(); }

    virtual bool concretize(Expression& _) const { (void) _; return is_concrete(); }
    
    virtual bool operator==(const Type& other) const {
        return name() == other.name();
    }
    
private:
    virtual bool cast_to_(const Type* _1, Expression& _2) const { (void) _1; (void) _2; return false; }
    
    const std::string name_;
    const Type* const wrapped_type_;
};

class CT_IO_WrappedType: public Type {
public:
    constexpr CT_IO_WrappedType(std::string_view name, const Type& wrapped_type)
    :name_(name),
     wrapped_type_(&wrapped_type) {}

    virtual bool is_pure() const { return false; }
    virtual bool is_concrete() const { return wrapped_type_->is_concrete(); }
    virtual bool is_complex() const { return true; }
    virtual std::string_view name() const { return name_; }
    virtual bool is_voidish() const { return wrapped_type_->is_voidish(); }

    virtual bool concretize(Expression& _) const { (void) _; return is_concrete(); }
    
    virtual bool operator==(const Type& other) const {
        return name() == other.name();
    }
    
private:
    virtual bool cast_to_(const Type* _1, Expression& _2) const { (void) _1; (void) _2; return false; }
    
    std::string_view name_;
    const Type* const wrapped_type_;
};

class IO_TypeConstructor {
public:
    // IO_TypeConstructor(std::string_view name)
    //  :name_(name) {}

    // std::string_view name_ = "IO";
    
    static IO_WrappedType apply(const Type& type_arg) {
        return IO_WrappedType{"IO " + type_arg.to_string(), type_arg};
    }

    static constexpr CT_IO_WrappedType ct_apply(std::string_view name, const Type& type_arg) {
        return CT_IO_WrappedType{name, type_arg};
    }
};

} // namespace Maps

#endif
