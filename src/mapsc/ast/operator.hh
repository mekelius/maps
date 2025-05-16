#ifndef __OPERATOR_HH
#define __OPERATOR_HH

#include "mapsc/source.hh"
#include "mapsc/ast/callable.hh"

namespace Maps {

class Operator: public Callable {
public:
    using Precedence = unsigned int;

    static constexpr Precedence MAX_PRECEDENCE = 1000;
    static constexpr Precedence MIN_PRECEDENCE = 0;

    enum class Fixity {
        binary,
        unary_prefix,
        unary_postfix
    };

    enum class Associativity {
        left
        // none, // not implemented, everything is left-associative
        // right, 
        // both,
    };

    struct Properties {
        Fixity fixity;
        Precedence precedence = 999;
        Associativity associativity = Associativity::left;
    };

    static Operator create_binary(std::string_view name, CallableBody body, Precedence precedence, 
        Associativity associativity, SourceLocation location) {

        return Operator{name, body, {Fixity::binary, precedence, associativity}, location};
    }

    static Operator create_binary(std::string_view name, CallableBody body, const Type& type, 
        Precedence precedence, Associativity associativity, SourceLocation location) {

        return Operator{name, body, type, {Fixity::binary, precedence, associativity}, location};
    }

    constexpr Operator(std::string_view name, const External external, const Type& type, 
        const Properties& operator_props)
    :Callable(name, external, type), operator_props_(operator_props) {}

    Operator(std::string_view name, CallableBody body, const Type& type, 
        Properties operator_props, SourceLocation location)
     :Callable(name, body, type, location), 
      operator_props_(operator_props) {}

    Operator(std::string_view name, CallableBody body, 
        Properties operator_props, SourceLocation location)
     :Callable(name, body, location), operator_props_(operator_props) {}

    Operator(const Operator& other) = default;
    Operator& operator=(const Operator& other) = default;
    virtual constexpr ~Operator() = default;

    bool is_operator() const { return true; }
    bool is_binary() const { return operator_props_.fixity == Fixity::binary; }
    bool is_unary() const { return operator_props_.fixity != Fixity::binary; }
    bool is_prefix() const { return operator_props_.fixity == Fixity::unary_prefix; }

    Precedence precedence() { return operator_props_.precedence; }
    Fixity fixity() const { return operator_props_.fixity; }

    Properties operator_props_;

private:
};

} // namespace Maps

#endif