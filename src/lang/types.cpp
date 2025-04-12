#include "types.hh"
#include <cassert>

namespace AST {

unsigned int Type::arity() const {
    if (const auto function_type = std::get_if<std::unique_ptr<FunctionType>>(&complex))
        return (*function_type)->arity();
    return 0;
}

bool Type::is_complex() const {
    return !std::holds_alternative<std::monostate>(complex);
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
            complex = std::make_unique<FunctionType>(*std::get<std::unique_ptr<FunctionType>>(rhs.complex));
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

    if (std::holds_alternative<std::unique_ptr<FunctionType>>(lhs.complex)) {
        auto& lhs_complex = *(std::get<std::unique_ptr<FunctionType>>(lhs.complex));
        auto& rhs_complex = *(std::get<std::unique_ptr<FunctionType>>(rhs.complex));
        // both are functiontypes
        return lhs_complex == rhs_complex;
    }

    assert(false && "unandled alternative in type operator ==");
    return false;
}


} // namespace AST