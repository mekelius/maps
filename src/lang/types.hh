#ifndef __TYPES_HH
#define __TYPES_HH

namespace AST {

// class for booleans we may or may not know yet
enum class DeferredBool {
    true_,
    false_,
    unknown,
};

struct Type {
    bool is_native = false;
    std::string_view name;
    DeferredBool is_numeric = DeferredBool::false_;
    DeferredBool is_integral = DeferredBool::false_;
};

constexpr Type Int {
    true,
    "Int",
    DeferredBool::true_,
    DeferredBool::true_,
};

constexpr Type Boolean {
    true,
    "Boolean",
    DeferredBool::false_,
    DeferredBool::false_,
};

constexpr Type String {
    true,
    "String",
    DeferredBool::false_,
    DeferredBool::false_,
};

constexpr Type Void {
    true,
    "Void",
    DeferredBool::false_,
    DeferredBool::false_,
};

constexpr Type Hole {
    false,
    "Hole",
    DeferredBool::unknown,
    DeferredBool::unknown,
};

// a number who's type hasn't yet been determined
constexpr Type NumberLiteral {
    false,
    "NumberLiteral",
    DeferredBool::true_,
    DeferredBool::unknown,
};

} // namespace AST
#endif