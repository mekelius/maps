#ifndef __FUNCTION_TYPE_HH
#define __FUNCTION_TYPE_HH

#include <array>
#include <optional>
#include <span>
#include <string>
#include <vector>
#include <sys/types.h>

#include "mapsc/types/casts.hh"
#include "mapsc/types/type.hh"

namespace Maps {

class FunctionType: public Type {
public:
    static std::string create_name(const Type* return_type, 
        std::span<const Type* const, std::dynamic_extent>param_types, bool is_pure) {
        
        if (param_types.size() == 0 && !is_pure) {
            return "Void -> " + std::string{return_type->name()};
        }

        std::string output = "";

        for (const Type* param_type: param_types) {
            output += param_type->name();
            output += is_pure ? " -> " : " => ";
        }

        output += return_type->name();

        return output;
    }

    virtual bool concretize(Expression&) const {
        return false;
    }

    virtual const Type* return_type() const = 0;
    virtual std::span<const Type* const> param_types() const = 0;
    virtual std::optional<const Type* const> param_type(uint param_index) const = 0;
    virtual bool is_voidish() const { return (arity() != 0 || return_type()->is_voidish()); }
    
    virtual bool is_complex() const { return true; }
    virtual bool is_function() const { return true; }

    virtual unsigned int arity() const { return param_types().size(); };

    virtual bool operator==(const Type& other) const {
        return name() == other.name();
    }

private:
    virtual bool cast_to_(const Type*, Expression&) const { return false; }
};

class RTFunctionType: public FunctionType {
public:
    RTFunctionType(const Type* return_type, 
        const std::vector<const Type*>& param_types, bool is_pure)
    :name_(FunctionType::create_name(return_type, std::span{param_types}, is_pure)),
     return_type_(return_type), 
     param_types_(param_types),
     is_pure_(is_pure) {}

    virtual bool is_pure() const { return is_pure_; }
    std::string_view name() const { return name_; }
    virtual const Type* return_type() const { return return_type_; }
    virtual std::span<const Type* const> param_types() const { return param_types_; }
    virtual std::optional<const Type* const> param_type(uint param_index) const {
        if (param_types_.size() < param_index + 1)
            return std::nullopt;

        return param_types_.at(param_index);
    };

    const std::string name_;
    const Type* return_type_;
    std::vector<const Type*> param_types_;
    bool is_pure_;
};

template <uint ARITY>
class CTFunctionType: public FunctionType {
public:
    constexpr CTFunctionType(std::string_view name, const Type* return_type, 
        const std::array<const Type*, ARITY>& param_types, bool is_pure)
    :name_(name),
     return_type_(return_type),
     param_types_(param_types),
     is_pure_(is_pure) {}

    virtual bool is_pure() const { return is_pure_; }
    std::string_view name() const { return name_; }
    virtual const Type* return_type() const { return return_type_; }
    virtual std::span<const Type* const> param_types() const { return param_types_; }
    virtual std::optional<const Type* const> param_type(uint param_index) const {
        if (param_types_.size() < param_index + 1)
            return std::nullopt;

        return param_types_.at(param_index);
    };

    const std::string_view name_;
    const Type* return_type_;
    const std::array<const Type*, ARITY> param_types_;
    bool is_pure_;
};    

} // namespace Maps

#endif