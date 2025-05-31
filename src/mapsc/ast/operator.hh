#ifndef __OPERATOR_HH
#define __OPERATOR_HH

#include "mapsc/source.hh"
#include "mapsc/ast/definition.hh"

namespace Maps {

class Operator: public DefinitionHeader {
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

    // static Operator create_binary(const std::string& name, DefinitionBody body, 
    //     Operator::Precedence precedence, Operator::Associativity associativity, bool is_top_level, 
    //     SourceLocation location) {

    //     return Operator{ name, body, 
    //         { Operator::Fixity::binary, precedence, associativity }, 
    //         is_top_level, location };
    // }

    // static Operator create_binary(const std::string& name, DefinitionBody body, const Type* type, 
    //     Operator::Precedence precedence, Operator::Associativity associativity, 
    //     bool is_top_level, SourceLocation location) {

    //     return Operator{ name, function, { Operator::Fixity::binary, precedence, associativity }, 
    //         location };
    // }

    static Operator create_binary(const std::string& name, DefinitionHeader* value, 
        Operator::Precedence precedence, Operator::Associativity associativity, 
        SourceLocation location) {

        return Operator { 
            name, 
            value, 
            Properties {
                Fixity::binary, 
                precedence, 
                associativity 
            }, 
            location 
        };
    }

    // ---------- CONSTRUCTORS ETC. ----------

    Operator(const std::string& name, DefinitionHeader* value,
        Operator::Properties operator_props, SourceLocation location)
    :DefinitionHeader(name, value->get_type(), location), 
     value_(value), 
     operator_props_(operator_props) {}

    Operator(const Operator& other) = default;
    Operator& operator=(const Operator& other) = default;
    virtual ~Operator() = default;

    // ---------- PUBLIC METHODS ----------

    virtual std::string node_type_string() const { return "operator"; }
    virtual std::string name_string() const { return name_; }
    virtual const Type* get_type() const { return value_->get_type(); };

    bool is_binary() const { return operator_props().fixity == Operator::Fixity::binary; }
    bool is_unary() const { return operator_props().fixity != Operator::Fixity::binary; }
    bool is_prefix() const { return operator_props().fixity == Operator::Fixity::unary_prefix; }

    Precedence precedence() const { return operator_props().precedence; }
    Fixity fixity() const { return operator_props().fixity; }

    bool is_operator() const { return true; }

    Properties operator_props() const {
        return operator_props_;
    };

    std::string name_;
    DefinitionHeader* header_;
    DefinitionHeader* value_;
    Operator::Properties operator_props_;

private:
};

} // namespace Maps

#endif