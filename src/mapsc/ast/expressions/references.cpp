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


Callable* Expression::reference_value() const {
    return std::get<Callable*>(value);
}

Callable* Expression::operator_reference_value() const {
    return std::get<Callable*>(value);
}

Expression* Expression::reference(AST_Store& store, Callable* callable, 
    SourceLocation location) {
    
    return store.allocate_expression(
        {ExpressionType::reference, callable, callable->get_type(), location});
}

std::optional<Expression*> Expression::reference(AST_Store& store, const Scope& scope, 
    const std::string& name, SourceLocation location) {
    
    if (auto callable = scope.get_identifier(name))
        return reference(store, *callable, location);

    return std::nullopt;
}

Expression* Expression::type_reference(AST_Store& store, const Type* type, 
    SourceLocation location) {
    
    return store.allocate_expression({ExpressionType::type_reference, type, &Void, location});
}

Expression Expression::operator_reference(Callable* callable, SourceLocation location) {
    assert(callable->is_operator() && "AST::create_operator_ref called with not an operator");

    ExpressionType expression_type;
    
    switch (dynamic_cast<Operator*>(callable)->fixity()) {
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

    return {expression_type, callable, callable->get_type(), location};
}

Expression* Expression::operator_reference(AST_Store& store, Callable* callable, 
    SourceLocation location) {
    
    return store.allocate_expression(operator_reference(callable, location));
}

void Expression::convert_to_reference(Callable* callable) {
    expression_type = ExpressionType::reference;
    value = callable;
    type = callable->get_type();
}

void Expression::convert_to_operator_reference(Callable* callable) {
    assert(callable->is_operator() && 
        "convert_to_operator_reference called with not an operator");

    auto op = dynamic_cast<Operator*>(callable);

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

    value = callable;
    type = callable->get_type();
}
    
Operator::Precedence get_operator_precedence(const Expression& operator_ref) {
    assert(operator_ref.expression_type == ExpressionType::binary_operator_reference && 
        "get_operator_precedence called with not a binary operator reference");

    return dynamic_cast<Operator*>(operator_ref.operator_reference_value())->precedence();
}

} // namespace Maps