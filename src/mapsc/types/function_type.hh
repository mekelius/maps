#ifndef __FUNCTION_TYPE_HH
#define __FUNCTION_TYPE_HH

#include <array>
#include <span>
#include <string>
#include <vector>
#include <sys/types.h>

#include "mapsc/types/casts.hh"
#include "mapsc/types/type.hh"

namespace Maps {

constexpr TypeTemplate Function_ {
    db_maybe,
    db_maybe,
};

class FunctionType: public Type {
public:
    // this is what is used as the basis for function specialization
    using HashableSignature = std::string;

    constexpr FunctionType(const Type* return_type, bool is_pure = false)
    :Type("Function", &Function_, not_castable, not_concretizable), 
     return_type_(return_type), is_pure_(is_pure) {}

    const Type* const return_type_;
    const bool is_pure_ = false;

    // just a synonym for to_string at the moment
    // TODO: memoize this
    HashableSignature hashable_signature() const {
        return to_string();
    }

    virtual const std::span<const Type* const> get_params() const = 0;

    virtual bool is_complex() const { return true; }
    virtual bool is_function() const { return true; }
    virtual bool is_pure() const { return is_pure_; }

    virtual unsigned int arity() const = 0;
};    

template <uint ARITY>
class CTFunctionType: public FunctionType {
public:
    constexpr CTFunctionType(const Type* return_type, 
        const std::array<const Type*, ARITY>& param_types, bool is_pure = false)
    :FunctionType(return_type, is_pure), 
     param_types_(param_types) {}

    const std::array<const Type*, ARITY> param_types_;

    // DO NOTE!: string representation is (currently) used as the basis for function overload specialization
    // IF YOU CREATE TYPES THAT HAVE IDENTICAL STRINGS THEIR FUNCTIONS WILL COLLIDE
    std::string to_string() const {
        if (arity() == 0) {
            return "Void -> " + return_type_->to_string();
        }

        std::string output = "";

        for (const Type* arg: param_types_) {
            output += arg->to_string();
            output += " -> ";
        }

        output += return_type_->to_string();

        return output;
    }

    // just a synonym for to_string at the moment
    // TODO: memoize this
    HashableSignature hashable_signature() const {
        return to_string();
    }

    virtual const std::span<const Type* const> get_params() const {
        return param_types_;
    }

    virtual unsigned int arity() const {
        return param_types_.size();
    }

    bool operator==(const CTFunctionType& other) {
        if (*dynamic_cast<const Type*>(this) != *dynamic_cast<const Type*>(&other))
            return false;

        if (this->return_type_ != other.return_type_)
            return false;

        return this->param_types_ == other.param_types_;
    }
};    

class RTFunctionType: public FunctionType {
public:
    RTFunctionType(const Type* return_type, 
        const std::vector<const Type*>& param_types, bool is_pure = false)
    :FunctionType(return_type, is_pure),
     param_types_(param_types) {}

    std::vector<const Type*> param_types_;

    // DO NOTE!: string rexistsepresentation is (currently) used as the basis for function overload specialization
    // IF YOU CREATE TYPES THAT HAVE IDENTICAL STRINGS THEIR FUNCTIONS WILL COLLIDE
    std::string to_string() const;

    // just a synonym for to_string at the moment
    // TODO: memoize this
    HashableSignature hashable_signature() const {
        return to_string();
    }

    virtual const std::span<const Type* const> get_params() const {
        return param_types_;
    }

    virtual unsigned int arity() const {
        return param_types_.size();
    }

    bool operator==(const RTFunctionType& other) {
        if (*dynamic_cast<const Type*>(this) != *dynamic_cast<const Type*>(&other))
            return false;

        if (this->return_type_ != other.return_type_)
            return false;

        return this->param_types_ == other.param_types_;
    }
};    

} // namespace Maps

#endif