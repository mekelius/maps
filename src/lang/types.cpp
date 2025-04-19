#include "types.hh"
#include <cassert>

namespace AST {

unsigned int Type::arity() const {
    if (const auto function_type = std::get_if<std::unique_ptr<FunctionTypeComplex>>(&complex))
        return (*function_type)->arity();
    return 0;
}

unsigned int Type::precedence() const {
    if (const auto function_type = std::get_if<std::unique_ptr<FunctionTypeComplex>>(&complex))
        return (*function_type)->precedence;
    return 0;
}

bool Type::is_complex() const {
    return !std::holds_alternative<std::monostate>(complex);
}

bool Type::is_operator() const {
    if (!is_complex())
        return false;

    return function_type()->is_operator;
}

bool Type::is_function() const {
    return (arity() > 0);
}

Type::Type(TypeTemplate* type_template): type_template(type_template) {
    complex = std::monostate{};
}

// copy constructor
Type::Type(const Type& rhs) {
    *this = rhs;
}

Type& Type::operator=(const Type& rhs) {
    if (this == &rhs)
        return *this;

    type_template = rhs.type_template;

    switch (rhs.complex.index()) {
        case 0:
            complex = std::monostate{};
            break;

        case 1: // function type
            complex = std::make_unique<FunctionTypeComplex>(*std::get<std::unique_ptr<FunctionTypeComplex>>(rhs.complex));
            break;
    }

    return *this;
}

bool operator==(const Type& lhs, const Type& rhs) {
    if (lhs.complex.index() != rhs.complex.index())
        return false;

    if (lhs.name() != rhs.name())
        return false;
        
    if (!lhs.is_complex())
        return true; // both are simple

    if (std::holds_alternative<std::unique_ptr<FunctionTypeComplex>>(lhs.complex)) {
        auto& lhs_complex = *(std::get<std::unique_ptr<FunctionTypeComplex>>(lhs.complex));
        auto& rhs_complex = *(std::get<std::unique_ptr<FunctionTypeComplex>>(rhs.complex));
        // both are functiontypes
        return lhs_complex == rhs_complex;
    }

    assert(false && "unhandled alternative in Type::==");
    return false;
}

// how could we make this more efficient
Type create_function_type(const Type& return_type, const std::vector<Type>& arg_types) {
    Type type = Type{ &Function_ };
    type.complex = std::make_unique<FunctionTypeComplex>(return_type, arg_types, false);
    return type;
}

Type create_binary_operator_type(const Type& return_type, const Type& lhs_type, 
    const Type& rhs_type, unsigned int precedence, Associativity associativity) {
    Type type = Type{ &Function_ };
    type.complex = std::make_unique<FunctionTypeComplex>(return_type, std::vector<Type>{lhs_type, rhs_type},
        true, Fixity::infix, precedence, associativity);
    return type;

}
Type create_unary_operator_type(const Type& return_type, const Type& arg_type, Fixity fixity) {
    Type type = Type{ &Function_ };
    type.complex = std::make_unique<FunctionTypeComplex>(return_type, std::vector<Type>{arg_type}, true, fixity);
    return type;
}

unsigned int get_precedence(const Type& type) {
    FunctionTypeComplex complex = *std::get<std::unique_ptr<FunctionTypeComplex>>(type.complex);
    assert(complex.is_operator && "get_precedence called with a non-operator type");

    return complex.precedence;
}

} // namespace AST