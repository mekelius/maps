#include "update_type.hh"

#include "mapsc/logging.hh"

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/procedures/inline.hh"

namespace Maps {

using Log = LogInContext<LogContext::type_checks>;

// Update type from an unkown type to a (hopefully) inferred type
bool update_type(DefinitionBody& definition) {
    assert(false && "not implemented");
}

bool update_type(Expression& expression) {
    assert(expression.type->is_unknown() && 
        "update_type called with an expresison that doesn't have an unknown type");

    switch (expression.expression_type) {
        case ExpressionType::known_value:
            Log::compiler_error(expression.location) << 
                "Encountered a known value with an unknown type" << Endl;
            return false;

        case ExpressionType::reference: {
            auto definition = expression.reference_value();
            if (definition->get_type()->is_unknown()) {
                assert(definition->body_ && "Encountered a definition with an unkown type and no body");
                
                if (!update_type(**definition->body_)) {
                    Log::error(expression.location) << "Updating reference type failed" << Endl;
                    return false;
                }

                assert(!definition->get_type()->is_unknown() && 
                    "Updating definition type didn't update into a known type");
            }

            // substitute the expression while we're at it
            if (definition->is_known_scalar_value()) {
                if (substitute_value_reference(expression)) {
                    Log::error(expression.location) << "Known value substitution failed" << Endl;
                    return false;
                }
            }

            expression.type = definition->get_type();
            return true;
        }

        case ExpressionType::known_value_reference:
            if (expression.reference_value()->get_type()->is_unknown()) {
                Log::compiler_error(expression.location) << 
                    "Encountered a known value with an unknown type" << Endl;
                return false;
            }

            Log::compiler_error(expression.location) << 
                "Known value reference type should have been set upon creation" << Endl;

            return substitute_value_reference(expression);

        case ExpressionType::call:
        case ExpressionType::partial_call:
        case ExpressionType::ternary_expression:
        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_binop_call_both:
        case ExpressionType::partially_applied_minus:
        case ExpressionType::missing_arg:
            assert(false && "not implemented");

        case ExpressionType::identifier:
        case ExpressionType::binary_operator_reference:
        case ExpressionType::postfix_operator_reference:
        case ExpressionType::prefix_operator_reference:
        case ExpressionType::operator_identifier:
        case ExpressionType::layer2_expression:
            Log::compiler_error(expression.location) << 
                "Unhandled expression: " << expression << " in update type" << Endl;    
            return false;

        default:
            Log::compiler_error(expression.location) << 
                "Illegal expression: " << expression << " in update type" << Endl;    
            return false;
    }
}

bool update_type(Statement& statement) {
    assert(false && "not implemented");
}

} // namespace Maps
