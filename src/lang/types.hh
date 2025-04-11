#ifndef __TYPES_HH
#define __TYPES_HH

#include <optional>
#include <variant>
#include <vector>
#include <string>
#include <string_view>

namespace AST {

// class for booleans we may or may not know yet
enum class DeferredBool {
    true_,
    false_,
    unknown,
};

// ----- COMPLEX TYPES -----

struct Type;

struct FunctionType {
    Type* return_type;
    std::vector<Type*> arg_types = {};

    unsigned int arity() const {
        return arg_types.size();
    }
};

using ComplexType = std::variant<std::monostate, FunctionType>;

struct Type {
    bool is_native = false;
    std::string_view name;
    DeferredBool is_numeric = DeferredBool::false_;
    DeferredBool is_integral = DeferredBool::false_;

    // this seems like a sane way to implement complex types without inheritance or 
    // whole-type variant
    ComplexType ct = std::monostate{};

    bool is_complex() const;    
    unsigned int arity() const;
};

// ----- SIMPLE TYPES -----

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