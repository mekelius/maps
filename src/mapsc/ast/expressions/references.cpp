#include "../expression.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

namespace Maps {

bool Expression::is_reference() const {
    switch (expression_type) {
        case ExpressionType::reference:
        case ExpressionType::type_reference:
        case ExpressionType::prefix_operator_reference:
        case ExpressionType::postfix_operator_reference:
        case ExpressionType::binary_operator_reference:
        case ExpressionType::type_operator_reference:
        case ExpressionType::type_constructor_reference:
            return true;

        default:
            return false;
    }
}


Definition* Expression::reference_value() const {
    return std::get<Definition*>(value);
}

Definition* Expression::operator_reference_value() const {
    return std::get<Definition*>(value);
}

Expression* Expression::reference(AST_Store& store, Definition* definition, 
    SourceLocation location) {
    
    return store.allocate_expression(
        {ExpressionType::reference, definition, definition->get_type(), location});
}

std::optional<Expression*> Expression::reference(AST_Store& store, const Scope& scope, 
    const std::string& name, SourceLocation location) {
    
    if (auto definition = scope.get_identifier(name))
        return reference(store, *definition, location);

    return std::nullopt;
}

Expression* Expression::type_reference(AST_Store& store, const Type* type, 
    SourceLocation location) {
    
    return store.allocate_expression({ExpressionType::type_reference, type, &Void, location});
}

Expression Expression::operator_reference(Definition* definition, SourceLocation location) {
    assert(definition->is_operator() && "AST::create_operator_ref called with not an operator");

    ExpressionType expression_type;
    
    switch (dynamic_cast<Operator*>(definition)->fixity()) {
        case Operator::Fixity::unary_prefix:
            expression_type = ExpressionType::prefix_operator_reference;
            break;
        case Operator::Fixity::unary_postfix:
            expression_type = ExpressionType::postfix_operator_reference;
            break;
        case Operator::Fixity::binary:
            expression_type = ExpressionType::binary_operator_reference;
            break;
    }

    return {expression_type, definition, definition->get_type(), location};
}

Expression* Expression::operator_reference(AST_Store& store, Definition* definition, 
    SourceLocation location) {
    
    return store.allocate_expression(operator_reference(definition, location));
}

void Expression::convert_to_reference(Definition* definition) {
    expression_type = ExpressionType::reference;
    value = definition;
    type = definition->get_type();
}

void Expression::convert_to_operator_reference(Definition* definition) {
    assert(definition->is_operator() && 
        "convert_to_operator_reference called with not an operator");

    auto op = dynamic_cast<Operator*>(definition);

    switch (op->fixity()) {
        case Operator::Fixity::binary:
            expression_type = ExpressionType::binary_operator_reference;
            break;
        case Operator::Fixity::unary_prefix:
            expression_type = ExpressionType::prefix_operator_reference;
            break;
        case Operator::Fixity::unary_postfix:
            expression_type = ExpressionType::postfix_operator_reference;
            break;
    }

    value = definition;
    type = definition->get_type();
}
    
Operator::Precedence get_operator_precedence(const Expression& operator_ref) {
    assert(operator_ref.expression_type == ExpressionType::binary_operator_reference && 
        "get_operator_precedence called with not a binary operator reference");

    return dynamic_cast<Operator*>(operator_ref.operator_reference_value())->precedence();
}

} // namespace Maps