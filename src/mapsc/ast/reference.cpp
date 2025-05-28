#include "reference.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/procedures/evaluate.hh"

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
    assert(std::holds_alternative<Definition*>(value) && 
        "Expression::reference_value called with not a reference to definition");
    return std::get<Definition*>(value);
}

const Type* Expression::type_reference_value() const {
    assert(std::holds_alternative<const Type*>(value) && 
        "Expression::reference_value called with not a reference to definition");
    return std::get<const Type*>(value);
}

Definition* Expression::operator_reference_value() const {
    return std::get<Definition*>(value);
}

Expression* create_reference(AST_Store& store, Definition* definition, 
    const SourceLocation& location) {
    
    if (definition->get_type()->is_function())
        return store.allocate_expression(
            {ExpressionType::reference, definition, definition->get_type(), location});

    return std::visit(overloaded{
        [&store, &definition, &location](const Expression* expression) {
            switch (expression->expression_type) {
                case ExpressionType::known_value:
                    return store.allocate_expression(
                        {ExpressionType::known_value_reference, definition, definition->get_type(), 
                            location});

                default:
                    return store.allocate_expression(
                        {ExpressionType::reference, definition, definition->get_type(), location});
            }
        },
        [&store, &definition, &location](auto) {
            return store.allocate_expression(
                {ExpressionType::reference, definition, definition->get_type(), location});
        }
    }, definition->const_body());
}

Expression* create_type_reference(AST_Store& store, const Type* type, 
    const SourceLocation& location) {
    
    return store.allocate_expression({ExpressionType::type_reference, type, &Void, location});
}

Expression create_operator_reference(Definition* definition, const SourceLocation& location) {
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

Expression* create_operator_reference(AST_Store& store, Definition* definition, 
    const SourceLocation& location) {
    
    return store.allocate_expression(create_operator_reference(definition, location));
}

void Expression::convert_to_reference(Definition* definition) {
    expression_type = definition->is_known_scalar_value() ? 
        ExpressionType::known_value_reference : ExpressionType::reference;
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
    
bool Expression::convert_by_value_substitution() {
    using Log = LogInContext<LogContext::inline_>;
    assert(expression_type == ExpressionType::known_value_reference &&
        "convert_by_value_substitution called on not a known value reference");

    assert(*type == *reference_value()->get_type() && 
        "attempting to substitute a value of a different type");

    Log::debug_extra("Attempting to substitute " + log_message_string(), location);

    auto new_value = evaluate(reference_value());
    if (!new_value) {
        Log::error("Substitution failed", location);
        return false;
    }

    expression_type = ExpressionType::known_value;
    value = std::visit([](auto value)->ExpressionValue { return {value}; }, *new_value);

    Log::debug_extra("Succesfully substituted, new expression: " + log_message_string(), location);

    return true;
}

Operator::Precedence get_operator_precedence(const Expression& operator_ref) {
    assert(operator_ref.expression_type == ExpressionType::binary_operator_reference && 
        "get_operator_precedence called with not a binary operator reference");

    return dynamic_cast<Operator*>(operator_ref.operator_reference_value())->precedence();
}

} // namespace Maps