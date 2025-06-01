#include "reference.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/definition_body.hh"
#include "mapsc/procedures/evaluate.hh"

namespace Maps {

DefinitionHeader* Expression::reference_value() const {
    assert(std::holds_alternative<DefinitionHeader*>(value) && 
        "Expression::reference_value called with not a reference to definition");
    return std::get<DefinitionHeader*>(value);
}

const Type* Expression::type_reference_value() const {
    assert(std::holds_alternative<const Type*>(value) && 
        "Expression::reference_value called with not a reference to definition");
    return std::get<const Type*>(value);
}

Operator* Expression::operator_reference_value() const {
    return dynamic_cast<Operator*>(std::get<DefinitionHeader*>(value));
}

Expression* create_reference(AST_Store& store, DefinitionHeader* definition, 
    const SourceLocation& location) {
    
    auto expression_type = definition->is_known_scalar_value() ?
        ExpressionType::known_value_reference : ExpressionType::reference; 
 
    return store.allocate_expression(
        {expression_type, definition, definition->get_type(), location});

    // if (definition->get_type()->is_function())
    //     return store.allocate_expression(
    //         {ExpressionType::reference, definition, definition->get_type(), location});

    // return std::visit(overloaded{
    //     [&store, &definition, &location](const Expression* expression) {
    //         switch (expression->expression_type) {
    //             case ExpressionType::known_value:
    //                 return store.allocate_expression(
    //                     {ExpressionType::known_value_reference, definition, definition->get_type(), 
    //                         location});

    //             default:
    //                 return store.allocate_expression(
    //                     {ExpressionType::reference, definition, definition->get_type(), location});
    //         }
    //     },
    //     [&store, &definition, &location](auto) {
    //         return store.allocate_expression(
    //             {ExpressionType::reference, definition, definition->get_type(), location});
    //     }
    // }, definition->const_body());
}

Expression* create_reference(AST_Store& store, DefinitionBody* definition, 
    const SourceLocation& location) {
    
    return create_reference(store, definition->header_, location);
}

std::optional<Expression*> create_reference(AST_Store& store, const Scope* scope, 
    const std::string& name, const SourceLocation& location){

    if (auto definition = scope->get_identifier(name))
        return create_reference(store, *definition, location);

    return std::nullopt;
}

Expression* create_type_reference(AST_Store& store, const Type* type, 
    const SourceLocation& location) {
    
    return store.allocate_expression({ExpressionType::type_reference, type, &Void, location});
}

Expression create_operator_reference(Operator* definition, const SourceLocation& location) {
    assert(definition->is_operator() && "AST::create_operator_ref called with not an operator");

    ExpressionType expression_type;
    
    switch (definition->fixity()) {
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

Expression* create_operator_reference(AST_Store& store, Operator* definition, 
    const SourceLocation& location) {
    
    return store.allocate_expression(create_operator_reference(definition, location));
}

void convert_to_reference(Expression& expression, DefinitionHeader* definition) {
    expression.expression_type = definition->is_known_scalar_value() ? 
        ExpressionType::known_value_reference : ExpressionType::reference;
    expression.value = definition;
    expression.type = definition->get_type();
}

void convert_to_operator_reference(Expression& expression, Operator* definition) {
    assert(definition->is_operator() && 
        "convert_to_operator_reference called with not an operator");

    auto op = definition;

    switch (op->fixity()) {
        case Operator::Fixity::binary:
            expression.expression_type = ExpressionType::binary_operator_reference;
            break;
        case Operator::Fixity::unary_prefix:
            expression.expression_type = ExpressionType::prefix_operator_reference;
            break;
        case Operator::Fixity::unary_postfix:
            expression.expression_type = ExpressionType::postfix_operator_reference;
            break;
    }

    expression.value = definition;
    expression.type = definition->get_type();
}
    
bool convert_by_value_substitution(Expression& expression) {
    using Log = LogInContext<LogContext::inline_>;
    assert(expression.expression_type == ExpressionType::known_value_reference &&
        "convert_by_value_substitution called on not a known value reference");

    assert(*expression.type == *expression.reference_value()->get_type() && 
        "attempting to substitute a value of a different type");

    Log::debug_extra(expression.location) << "Attempting to substitute " << expression;

    if (!expression.reference_value()->body_) {
        Log::compiler_error(expression.location) << 
            "Attempted to substitute a definition without a body";
        return false;
    }

    auto new_value = evaluate(**expression.reference_value()->body_);
    if (!new_value) {
        Log::error(expression.location) << "Substitution failed";
        return false;
    }

    expression.expression_type = ExpressionType::known_value;
    expression.value = std::visit([](auto value)->ExpressionValue { return {value}; }, *new_value);

    Log::debug_extra(expression.location) << "Succesfully substituted, new expression: " << expression;

    return true;
}

Operator::Precedence get_operator_precedence(const Expression& operator_ref) {
    assert(operator_ref.expression_type == ExpressionType::binary_operator_reference && 
        "get_operator_precedence called with not a binary operator reference");

    return dynamic_cast<Operator*>(operator_ref.operator_reference_value())->precedence();
}

} // namespace Maps