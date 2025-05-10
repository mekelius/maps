#ifndef __FUNCTION_TYPE_HH
#define __FUNCTION_TYPE_HH

#include <string>
#include <vector>

#include "mapsc/types/type.hh"

namespace Maps {

// !! this struct has way too much stuff
// maybe inheritance is the way
class FunctionType: public Type {
public:
    // this is what is used as the basis for function specialization
    using HashableSignature = std::string;

    FunctionType(const ID id, const TypeTemplate* type_template, const Type* return_type, 
        const std::vector<const Type*>& arg_types, bool is_pure = false);

    const Type* return_type_;
    std::vector<const Type*> param_types_;
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
        return param_types_.size();
    }

    bool operator==(const FunctionType& other) {
        if (*dynamic_cast<const Type*>(this) != *dynamic_cast<const Type*>(&other))
            return false;

        if (this->return_type_ != other.return_type_)
            return false;

        return this->param_types_ == other.param_types_;
    }
};    

} // namespace Maps

#endif