#ifndef __CALLABLE_HH
#define __CALLABLE_HH

#include <variant>
#include <optional>
#include <string>

#include "mapsc/source.hh"

namespace Maps {

class Type;
class Expression;
struct Statement;
struct Builtin;
struct Operator;

using CallableBody = std::variant<std::monostate, Expression*, Statement*, Builtin*>;

/**
 * Callables represent either expressions or statements, along with extra info like
 * holding types for statements. A bit convoluted but we'll see.
 * The source location is needed on every callable that is not a built-in
 * 
 * NOTE: if it's anonymous, it needs a source location
 */
class Callable {
public:
    Callable(CallableBody body, const std::string& name,
        std::optional<SourceLocation> location = std::nullopt);
    Callable(CallableBody body, std::optional<SourceLocation> location); // create anonymous callable

    Callable(const Callable& other) = default;
    Callable& operator=(const Callable& other) = default;
    ~Callable() = default;

    // tries to remove braces etc.
    void attempt_simplify();
    // the parameter must be a call expression
    void attempt_inline(Expression* call);

    CallableBody body;
    std::string name;
    std::optional<SourceLocation> location;
    std::optional<Operator*> operator_props;

    std::optional<const Type*> declared_type;

    // since statements don't store types, we'll have to store them here
    // if the body is an expression, the type will just mirror it's type
    const Type* get_type() const;
    void set_type(const Type& type);

    std::optional<const Type*> get_declared_type() const;
    bool set_declared_type(const Type& type);

    // checks if the name is an operator style name
    bool is_operator() const;
    bool is_binary_operator() const;
    bool is_unary_operator() const;

    bool operator==(const Callable&) const = default;

private:
    std::optional<const Type*> type_;
};

} // namespace Maps

#endif