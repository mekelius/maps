#ifndef __OPERATOR_HH
#define __OPERATOR_HH

#include "mapsc/source.hh"
#include "mapsc/ast/definition.hh"

namespace Maps {

class Operator {
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

    bool is_binary() const { return operator_props().fixity == Operator::Fixity::binary; }
    bool is_unary() const { return operator_props().fixity != Operator::Fixity::binary; }
    bool is_prefix() const { return operator_props().fixity == Operator::Fixity::unary_prefix; }

    Precedence precedence() { return operator_props().precedence; }
    Fixity fixity() const { return operator_props().fixity; }

    virtual Properties operator_props() const = 0;
};

class RT_Operator: public RT_Definition, public Operator {
public:
    static RT_Operator create_binary(std::string_view name, DefinitionBody body, 
        Operator::Precedence precedence, Operator::Associativity associativity, bool is_top_level, 
        SourceLocation location) {

        return RT_Operator{ name, body, 
            { Operator::Fixity::binary, precedence, associativity }, 
            is_top_level, location };
    }

    static RT_Operator create_binary(std::string_view name, DefinitionBody body, const Type* type, 
        Operator::Precedence precedence, Operator::Associativity associativity, 
        bool is_top_level, SourceLocation location) {

        return RT_Operator{ name, body, type, 
            { Operator::Fixity::binary, precedence, associativity }, 
                is_top_level, location };
    }

    RT_Operator(std::string_view name, const External external, const Type* type, 
        const Operator::Properties& operator_props)
    :RT_Definition(name, external, type),
     operator_props_(operator_props) {}

    RT_Operator(std::string_view name, DefinitionBody body, const Type* type, 
        Operator::Properties operator_props, bool is_top_level, SourceLocation location)
    :RT_Definition(name, body, type, is_top_level, location), 
     operator_props_(operator_props) {}

    RT_Operator(std::string_view name, DefinitionBody body, 
        Operator::Properties operator_props, bool is_top_level, SourceLocation location)
    :RT_Definition(name, body, is_top_level, location), 
     operator_props_(operator_props) {}

    RT_Operator(const RT_Operator& other) = default;
    RT_Operator& operator=(const RT_Operator& other) = default;
    virtual ~RT_Operator() = default;

    bool is_operator() const { return true; }

    virtual Properties operator_props() const {
        return operator_props_;
    };

    Operator::Properties operator_props_;

private:
};

} // namespace Maps

#endif