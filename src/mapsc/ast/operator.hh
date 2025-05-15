#ifndef __OPERATOR_HH
#define __OPERATOR_HH

#include "mapsc/source.hh"
#include "mapsc/ast/callable.hh"

namespace Maps {

using Precedence = unsigned int;

constexpr Precedence MAX_OPERATOR_PRECEDENCE = 1000;
constexpr Precedence MIN_OPERATOR_PRECEDENCE = 0;

enum class BinaryFixity {
    infix,
    none,
    // prefix, // not implemented
    // postfix,
};

enum class UnaryFixity {
    prefix,
    none,
    postfix
};

enum class Associativity {
    left,
    // none, // not implemented, everything is left-associative
    // right, 
    // both,
};

struct OperatorProps {
    constexpr static OperatorProps Binary(Precedence precedence, Associativity associativity) {
        return OperatorProps{UnaryFixity::none, BinaryFixity::infix, precedence, associativity};
    }

    constexpr static OperatorProps Unary() {
        return OperatorProps{UnaryFixity::prefix};
    }

    UnaryFixity unary_fixity = UnaryFixity::prefix;
    BinaryFixity binary_fixity = BinaryFixity::infix;
    
    Precedence precedence = 999;
    Associativity associativity = Associativity::left;

    bool is_unary() const { return unary_fixity != UnaryFixity::none; };
    bool is_binary() const { return binary_fixity != BinaryFixity::none; };
};

class Operator: public Callable {
public:
    static Operator create_binary(std::string_view name, CallableBody body, Precedence precedence, 
        Associativity associativity, SourceLocation location) {

        return Operator{name, body, OperatorProps::Binary(precedence, associativity), location};
    }

    static Operator create_binary(std::string_view name, CallableBody body, const Type& type, 
        Precedence precedence, Associativity associativity, SourceLocation location) {

        return Operator{name, body, type, OperatorProps::Binary(precedence, associativity), location};
    }

    constexpr Operator(std::string_view name, const External external, const Type& type, 
        const OperatorProps& operator_props)
    :Callable(name, external, type), operator_props_(operator_props) {}

    Operator(std::string_view name, CallableBody body, const Type& type, 
        OperatorProps operator_props, SourceLocation location)
     :Callable(name, body, type, location), 
      operator_props_(operator_props) {}

    Operator(std::string_view name, CallableBody body, 
        OperatorProps operator_props, SourceLocation location)
     :Callable(name, body, location), operator_props_(operator_props) {}

    Operator(const Operator& other) = default;
    Operator& operator=(const Operator& other) = default;
    virtual constexpr ~Operator() = default;

    bool is_operator() const {
        return true;
    }

    bool is_binary_operator() const {
        return operator_props_.is_binary();
    }

    bool is_unary_operator() const {
        return operator_props_.is_unary();
    }

    Precedence get_precedence() {
        return operator_props_.precedence;
    }

    OperatorProps operator_props_;

private:
};

} // namespace Maps

#endif