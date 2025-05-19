#ifndef __CALLABLE_HH
#define __CALLABLE_HH

#include <optional>
#include <string_view>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/types/type_defs.hh"
#include "common/std_visit_helper.hh"


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
    virtual std::string_view name() const = 0;
    virtual std::string to_string() const { return std::string{name()}; };
    virtual CallableBody const_body() const = 0;
    virtual const SourceLocation& location() const = 0;
    virtual const Type* get_type() const = 0;
    virtual std::optional<const Type*> get_declared_type() const = 0;

    bool is_undefined() const {
        return std::holds_alternative<Undefined>(const_body());
    }

    bool is_empty() const;

    virtual bool is_operator() const { return false; }
    virtual bool is_const() const { return false; }

    virtual bool operator==(const Callable& other) const = 0;
};

class RT_Callable: public Callable {
public:
    // creates a new dummy callable suitable for unit testing
    static RT_Callable testing_callable(const Type* type = &Hole); 

    RT_Callable(std::string_view name, External external, const Type& type)
    :name_(name), body_(external), location_(EXTERNAL_SOURCE_LOCATION), type_(&type) {
    }

    RT_Callable(std::string_view name, CallableBody body, SourceLocation location);
    RT_Callable(CallableBody body, SourceLocation location); // create anonymous callable

    // anonymous callables
    RT_Callable(std::string_view name, CallableBody body, const Type& type, SourceLocation location);
    RT_Callable(CallableBody body, const Type& type, SourceLocation location);

    RT_Callable(const RT_Callable& other) = default;
    RT_Callable& operator=(const RT_Callable& other) = default;
    virtual constexpr ~RT_Callable() = default;

    // ----- OVERRIDES ------
    virtual std::string_view name() const { return name_; }
    virtual std::string to_string() const { return name_; }
    virtual CallableBody const_body() const { return body_; }
    virtual CallableBody& body() { return body_; }
    virtual const SourceLocation& location() const { return location_; }

    // since statements don't store types, we'll have to store them here
    // if the body is an expression, the type will just mirror it's type
    virtual const Type* get_type() const;
    virtual std::optional<const Type*> get_declared_type() const;


    void set_type(const Type& type);
    bool set_declared_type(const Type& type);

    virtual bool operator==(const Callable& other) const {
        if (this == &other)
            return true;

        if (const_body() == other.const_body())
            return true;

        return false;
    }

private:
    std::string name_;
    CallableBody body_;
    SourceLocation location_;
    std::optional<const Type*> type_;
    std::optional<const Type*> declared_type_;
};

} // namespace Maps

#endif