#ifndef __TYPE_REGISTRY_HH
#define __TYPE_REGISTRY_HH

#include <cstddef>
#include <span>
#include <memory>
#include <map>
#include <concepts>
#include <vector>
#include <string>
#include <optional>
#include <ranges>

#include <initializer_list>

#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"

namespace Maps {

class FunctionType;

// class for holding the shared type information such as traits
// See identifying_types text file for better description
class TypeStore {
public:
    TypeStore(const std::span<const Type* const> builtin_simple_types = BUILTIN_TYPES,
        const std::span<const FunctionType* const> builtin_function_types = BUILTIN_FUNCTION_TYPES);

    // checks if the TypeRegistry contains any types besides the builtin ones
    bool empty() const;
    // builtin types don't count
    size_t size() const;

    // TODO: move this to be private, callers should use get instead
    std::optional<const Type*> create_type(const std::string& name);
    
    std::optional<const Type*> get(const auto identifier) const {
        const auto it = types_by_identifier_.find(identifier);
        if (it == types_by_identifier_.end())
            return std::nullopt;

        return it->second;
    };

    template <std::ranges::forward_range R = std::initializer_list<const Type*>>
        requires std::convertible_to<std::ranges::range_value_t<R>, const Type*>
    const FunctionType* get_function_type(const Type* return_type, R arg_types, bool is_pure) {
        std::string signature = make_function_signature(return_type, arg_types, is_pure);
        auto existing_it = types_by_structure_.find(signature);

        // if type with this structure exists, just return that
        if (existing_it != types_by_structure_.end())
            return dynamic_cast<const FunctionType*>(existing_it->second);
        
        // else create that type
        return create_function_type(signature, return_type, arg_types, is_pure);
    }

    template <std::ranges::forward_range R>
    std::string make_function_signature(const Type* return_type, R arg_types, bool is_pure) const {
        // nullary pure function is just a value
        if (arg_types.size() == 0 && is_pure)
            return std::string{return_type->name()};

        // nullary impure function gets the special type =>return_type
        if (arg_types.size() == 0)
            return "=>" + std::string{return_type->name()};

        std::string signature = "";
        bool first = true;
        for (auto arg_type: arg_types) {
            if (!first)
                signature += "-";
            first = false;
            signature += std::string{arg_type->name()};
        }

        signature += is_pure ? "-" : "=";

        return signature + std::string{return_type->name()};
    }

private:
    template <std::ranges::forward_range R>
    const FunctionType* create_function_type(const std::string& signature,
        const Type* return_type, R arg_types, bool is_pure = true) {

        std::unique_ptr<const Type> up = 
            make_unique<const RTFunctionType>(return_type, arg_types, is_pure);
        types_.push_back(std::move(up));
        auto raw_ptr = types_.back().get();

        types_by_structure_.insert({signature, raw_ptr});

        return dynamic_cast<const FunctionType*>(raw_ptr);
    }

    std::map<std::string, const Type*, std::less<>> types_by_identifier_ = {};
    std::map<std::string, const Type*, std::less<>> types_by_structure_ = {};

    // we need two different vectors, since the builtin types need to be accessable by id as well
    std::vector<std::unique_ptr<const Type>> types_ = {};
    
    // std::vector<std::unique_ptr<TypeConstructor>> type_constructors_ = {};
    // std::map<std::string, const TypeConstructor*> typeconstructors_by_identifier = {};
};

} // namespace Maps

#endif