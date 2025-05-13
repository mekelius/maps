#ifndef __OPERATOR_HH
#define __OPERATOR_HH

#include "mapsc/source.hh"

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
    // postfix, // not implemented
    // both_left_biased,
    // both_right_biased,
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

} // namespace Maps

#endif