#ifndef __TYPES_HH
#define __TYPES_HH

#include <optional>
#include <variant>

namespace AST {

// class for booleans we may or may not know yet
enum class DeferredBool {
    true_,
    false_,
    unknown,
};

struct FunctionType;
using ComplexType = std::variant<std::monostate, FunctionType>;

struct Type {
    bool is_native = false;
    std::string_view name;
    DeferredBool is_numeric = DeferredBool::false_;
    DeferredBool is_integral = DeferredBool::false_;

    // this seems like a sane way to implement complex types without inheritance or 
    // whole-type variant
    ComplexType ct = std::monostate{};

    bool is_complex() const {
        return !std::holds_alternative<std::monostate>(ct);
    }
    
    unsigned int arity() const {
        if (const auto function_type = std::get_if<FunctionType>(&ct))
            return function_type->arity();
        return 0;
    }
};

struct FunctionType {
    Type* return_type;
    std::vector<Type*> parameter_types = {};

    unsigned int arity() const {
        return parameter_types.size();
    }
};

const Type Int {
    true,
    "Int",
    DeferredBool::true_,
    DeferredBool::true_,
};

const Type Boolean {
    true,
    "Boolean",
    DeferredBool::false_,
    DeferredBool::false_,
};

const Type String {
    true,
    "String",
    DeferredBool::false_,
    DeferredBool::false_,
};

const Type Void {
    true,
    "Void",
    DeferredBool::false_,
    DeferredBool::false_,
};

const Type Hole {
    false,
    "Hole",
    DeferredBool::unknown,
    DeferredBool::unknown,
};

// a number who's type hasn't yet been determined
const Type NumberLiteral {
    false,
    "NumberLiteral",
    DeferredBool::true_,
    DeferredBool::unknown,
};

} // namespace AST
#endif