#ifndef __OPERATOR_HH
#define __OPERATOR_HH

#include "mapsc/source_location.hh"
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

    static Operator create_binary(std::string name, DefinitionHeader* value, 
        Operator::Precedence precedence, Operator::Associativity associativity, 
        SourceLocation location);

    // ---------- CONSTRUCTORS ETC. ----------

    constexpr Operator(std::string_view name, const DefinitionHeader* value,
        Operator::Properties operator_props, SourceLocation location)
    :DefinitionHeader(DefinitionType::operator_def, name, value->get_type(), std::move(location)),
     value_(value), 
     operator_props_(std::move(operator_props)) {}

    Operator(const Operator& other) = default;
    Operator& operator=(const Operator& other) = default;
    virtual constexpr ~Operator() = default;

    // ---------- PUBLIC METHODS ----------

    virtual std::string node_type_string() const { return "operator"; }
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

    const DefinitionHeader* value_;
    Operator::Properties operator_props_;

private:
};

class RT_Operator: public Operator {
public:
    RT_Operator(std::string name, const DefinitionHeader* value,
        Operator::Properties operator_props, SourceLocation location)
    :Operator("", value, std::move(operator_props), std::move(location)), 
     name_string_(std::move(name)) {
        name_ = name_string_;
    }

    virtual ~RT_Operator() = default;
    // Move and copy constructors need to update the name_
    RT_Operator(const RT_Operator& other) noexcept
    :Operator("", other.value_, other.operator_props_, other.location_), 
     name_string_(other.name_string_) {
        name_ = name_string_;
    }
    RT_Operator(RT_Operator&& other) noexcept
    :Operator("", other.value_, std::move(other.operator_props_), std::move(other.location_)), 
     name_string_(std::move(other.name_string_)) {
        name_ = name_string_;
    }
    // can't be bothered to figure these out now
    RT_Operator& operator=(RT_Operator&&) = delete; 
    RT_Operator& operator=(const RT_Operator&) = delete;

private:
    std::string name_string_;
};


Operator* create_binary_operator(AST_Store& ast_store, std::string name, DefinitionHeader* value, 
    Operator::Precedence precedence, Operator::Associativity associativity, 
    SourceLocation location);

Operator* create_binary_operator(AST_Store& ast_store, std::string name, DefinitionHeader* value, 
    Operator::Precedence precedence, SourceLocation location);

} // namespace Maps


#endif