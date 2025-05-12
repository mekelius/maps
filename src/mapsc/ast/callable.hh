#ifndef __CALLABLE_HH
#define __CALLABLE_HH

#include <variant>
#include <optional>
#include <string>

#include "mapsc/source.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/operator.hh"

namespace Maps {

class Type;
struct Expression;
struct Statement;
struct Builtin;
struct OperatorProps;

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

    Callable(const std::string& name, CallableBody body, SourceLocation location);
    Callable(CallableBody body, SourceLocation location); // create anonymous callable

    // anonymous callables
    Callable(const std::string& name, CallableBody body, const Type& type, SourceLocation location);
    Callable(CallableBody body, const Type& type, SourceLocation location);

    Callable(const Callable& other) = default;
    Callable& operator=(const Callable& other) = default;
    virtual ~Callable() = default;

    std::string name_;
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

    bool operator==(const Callable&) const = default;

private:
    std::optional<const Type*> declared_type_;
    std::optional<const Type*> type_;
};

class Operator: public Callable {
public:
    static Operator create_binary(const std::string& name, CallableBody body, 
        Precedence precedence, Associativity associativity, SourceLocation location);
    
    static Operator create_binary(const std::string& name, CallableBody body, const Type& type,
        Precedence precedence, Associativity associativity, SourceLocation location);

    Operator(const std::string& name, CallableBody body, const Type& type, 
        OperatorProps operator_props, SourceLocation location)
     :Callable(name, body, type, location), 
      operator_props_(operator_props) {}

    Operator(const std::string& name, CallableBody body, 
        OperatorProps operator_props, SourceLocation location)
     :Callable(name, body, location), operator_props_(operator_props) {}

    Operator(const Operator& other) = default;
    Operator& operator=(const Operator& other) = default;
    virtual ~Operator() = default;

    virtual bool is_operator() const;
    virtual bool is_binary_operator() const;
    virtual bool is_unary_operator() const;

    Precedence get_precedence();

    OperatorProps operator_props_;

private:
};

} // namespace Maps

#endif