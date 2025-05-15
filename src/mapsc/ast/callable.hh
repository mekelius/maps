#ifndef __CALLABLE_HH
#define __CALLABLE_HH

#include <optional>
#include <string_view>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/types/type_defs.hh"

namespace Maps {

class Type;
struct Expression;
struct Statement;

struct External {
    template<typename T>
    bool operator==(T&) const { return false; }
};

struct Undefined {
    template<typename T>
    bool operator==(T&) const { return false; }
};

using CallableBody = std::variant<Undefined, Expression*, Statement*, External>;

/**
 * Callables represent either expressions or statements, along with extra info like
 * holding types for statements. A bit convoluted but we'll see.
 * The source location is needed on every callable that is not a built-in
 * 
 * NOTE: if it's anonymous, it needs a source location
 */
class Callable {
public:
    // creates a new dummy callable suitable for unit testing
    static Callable testing_callable(const Type* type = &Hole); 

    constexpr Callable(std::string_view name, External external, const Type& type)
    :name_(name), body_(external), location_(EXTERNAL_SOURCE_LOCATION), type_(&type) {

    }

    Callable(std::string_view name, CallableBody body, SourceLocation location);
    Callable(CallableBody body, SourceLocation location); // create anonymous callable

    // anonymous callables
    Callable(std::string_view name, CallableBody body, const Type& type, SourceLocation location);
    Callable(CallableBody body, const Type& type, SourceLocation location);

    Callable(const Callable& other) = default;
    Callable& operator=(const Callable& other) = default;
    virtual constexpr ~Callable() = default;

    std::string_view name_;
    CallableBody body_;
    SourceLocation location_;

    // since statements don't store types, we'll have to store them here
    // if the body is an expression, the type will just mirror it's type
    const Type* get_type() const;
    void set_type(const Type& type);

    std::optional<const Type*> get_declared_type() const;
    bool set_declared_type(const Type& type);

    // checks if the name is an operator style name
    bool is_undefined() const;
    virtual bool is_operator() const;
    virtual bool is_binary_operator() const;
    virtual bool is_unary_operator() const;
    virtual bool is_const() const { return false; }

    bool operator==(const Callable&) const = default;

private:
    std::optional<const Type*> declared_type_;
    std::optional<const Type*> type_;
};

} // namespace Maps

#endif