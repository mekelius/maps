#ifndef __TYPE_CONSTRUCTOR_HH
#define __TYPE_CONSTRUCTOR_HH

#include <tuple>
#include <vector>
#include <string>
#include <optional>
#include <array>

namespace Maps {

class Type;
class TypeRegistry;

class TypeConstructor {
public:
    using TypeArg = std::tuple<const Type*, std::optional<std::string>>;

    TypeConstructor(const std::string& name, int arity);

    virtual ~TypeConstructor() = default;
    TypeConstructor(TypeConstructor&) = delete;
    TypeConstructor& operator=(TypeConstructor&) = delete;

    virtual const Type* make_type(TypeRegistry& type_registry, std::vector<TypeArg>&& args, 
        std::string* name = nullptr) = 0;
    
    static constexpr int ARITY_N = -1;
    
    std::string name_;
    const int arity_ = 1;
};
    
class PureFunctionConstructor: public TypeConstructor {
public:
    PureFunctionConstructor();
    const Type* make_type(TypeRegistry& type_registry, std::vector<TypeArg>&& args, 
        std::string* name = nullptr);
};
static const PureFunctionConstructor PureFunction{};

class ImpureFunctionConstructor: public TypeConstructor {
public:
    ImpureFunctionConstructor();
    const Type* make_type(TypeRegistry& type_registry, std::vector<TypeArg>&& args, 
        std::string* name = nullptr);
};
static const PureFunctionConstructor ImpureFunction{};

// static constexpr TypeConstructor optional;
// static constexpr TypeConstructor tuple;
// static constexpr TypeConstructor union_tc;
// static constexpr TypeConstructor sequence;

static const std::array<const TypeConstructor*, 2> BUILTIN_TYPECONSTRUCTORS = {
    &PureFunction, 
    &ImpureFunction, 
};

} // namespace Maps

#endif