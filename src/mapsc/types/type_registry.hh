#ifndef __TYPE_REGISTRY_HH
#define __TYPE_REGISTRY_HH

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>

#include "mapsc/types/type.hh"
#include "mapsc/types/function_type.hh"

namespace Maps {

// class for holding the shared type information such as traits
// See identifying_types text file for better description
class TypeRegistry {
public:
    TypeRegistry();

    // checks if the TypeRegistry contains any types besides the builtin ones
    bool empty() const;
    // builtin types don't count
    size_t size() const;

    // TODO: move this to be private, callers should use get instead
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