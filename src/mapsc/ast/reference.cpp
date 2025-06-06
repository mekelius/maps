#include "reference.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/definition_body.hh"
#include "mapsc/ast/call_expression.hh"
#include "mapsc/procedures/evaluate.hh"

namespace Maps {

const DefinitionHeader* Expression::reference_value() const {
    assert(std::holds_alternative<const DefinitionHeader*>(value) && 
        "Expression::reference_value called with not a reference to definition");
    return std::get<const DefinitionHeader*>(value);
}

const Type* Expression::type_reference_value() const {
    assert(std::holds_alternative<const Type*>(value) && 
        "Expression::reference_value called with not a reference to definition");
    return std::get<const Type*>(value);
}

const Operator* Expression::operator_reference_value() const {
    return dynamic_cast<const Operator*>(std::get<const DefinitionHeader*>(value));
}

Expression* create_reference(AST_Store& store, const DefinitionHeader* definition, 
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

Expression create_operator_reference(const Operator* definition, const SourceLocation& location) {
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

Expression* create_operator_reference(AST_Store& store, const Operator* definition, 
    const SourceLocation& location) {
    
    return store.allocate_expression(create_operator_reference(definition, location));
}

void convert_to_reference(Expression& expression, const DefinitionHeader* definition) {
    expression.expression_type = definition->is_known_scalar_value() ? 
        ExpressionType::known_value_reference : ExpressionType::reference;
    expression.value = definition;
    expression.type = definition->get_type();
}

void convert_to_operator_reference(Expression& expression, const Operator* definition) {
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

    Log::debug_extra(expression.location) << "Attempting to substitute " << expression << Endl;

    if (!expression.reference_value()->body_) {
        Log::compiler_error(expression.location) << 
            "Attempted to substitute a definition without a body";
        return false;
    }

    auto new_value = evaluate(**expression.reference_value()->body_);
    if (!new_value) {
        Log::error(expression.location) << "Substitution failed" << Endl;
        return false;
    }

    expression.expression_type = ExpressionType::known_value;
    expression.value = std::visit([](auto value)->ExpressionValue { return {value}; }, *new_value);

    Log::debug_extra(expression.location) << "Succesfully substituted, new expression: " << expression << Endl;

    return true;
}

Operator::Precedence get_operator_precedence(const Expression& operator_ref, bool from_left) {
    using Log = LogNoContext;

    switch (operator_ref.expression_type) {
        case ExpressionType::binary_operator_reference:
            return dynamic_cast<const Operator*>(operator_ref.operator_reference_value())->precedence();

        case ExpressionType::partial_binop_call_left: {
            if (!from_left) {
                Log::compiler_error(operator_ref.location) << 
                    "get_operator_precedence called from wrong side on " << operator_ref << Endl;
                assert(false && "get_operator_precedence called from wrong side");
                break;
            }

            auto [callee, args] = operator_ref.call_value();
            assert(callee->is_operator() && "encountered partial binop call left with not an operator as callee");
            auto op = dynamic_cast<const Operator*>(callee);
            assert(op);

            return op->precedence();
        }
        case ExpressionType::partial_binop_call_right:{
            if (from_left) {
                Log::compiler_error(operator_ref.location) << 
                    "get_operator_precedence called from wrong side on " << operator_ref << Endl;
                assert(false && "get_operator_precedence called from wrong side");
                break;
            }

            auto [callee, args] = operator_ref.call_value();
            assert(callee->is_operator() && 
                "encountered partial binop call left with not an operator as callee");
            auto op = dynamic_cast<const Operator*>(callee);
            assert(op);

            return op->precedence();
        }
        case ExpressionType::partial_binop_call_both:
            assert(false && "not implemented");
            break;

        case ExpressionType::partially_applied_minus:
            assert(false && 
                "Partially applied minus should have been converted to partial binop call first");
            break;  

        default:
            assert(false && "get_operator_precedence called with not a binary operator reference");
            break;
    }
}

} // namespace Maps