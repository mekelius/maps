#include "../expression.hh"

#include <cassert>
#include <string>

#include "mapsc/logging.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/builtins.hh"

#include "mapsc/ast/expression_properties.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/call_expression.hh"
#include "mapsc/ast/lambda.hh"
#include "mapsc/types/function_type.hh"

using std::optional, std::nullopt, std::to_string;


namespace Maps {

using Log = LogInContext<LogContext::type_casts>;

optional<Expression*> Expression::cast_to(CompilationState& state, const Type* target_type, 
    const SourceLocation& type_declaration_location) {
    
    using Log = LogInContext<LogContext::type_checks>;

    switch (expression_type) {
        case NON_CASTABLE_EXPRESSION:
            Log::debug(type_declaration_location) << "Expression " << *this << " in not castable";
            return nullopt;

        case ExpressionType::known_value:
            // as a special case, every type can be casted into a const function of itself
            if (target_type->is_function()) {
                Log::debug_extra(type_declaration_location) << "Attempting to cast " << *this << 
                    " to const lambda " << *target_type;
                auto function_type = dynamic_cast<const FunctionType*>(target_type);
                if (*function_type->return_type() != *type) {
                    Log::debug(type_declaration_location) << "Could not cast " << *this << " to " << 
                        *target_type;
                    return nullopt;
                }

                auto [expression, definition] = create_const_lambda(state, this, 
                    function_type->param_types(), type_declaration_location, 
                    function_type->is_pure());
                
                Log::debug_extra(type_declaration_location) << 
                    "Succesfully casted " << *this << " into " << *type;

                Log::debug_extra(type_declaration_location) << "Created lambda wrapper " << *definition;
                return expression;
            }

            if (type->cast_to(target_type, *this)) {
                Log::debug_extra(type_declaration_location) << "Casted " << *this << " to " << 
                    *target_type;
                return this;
            }
                
            Log::debug(type_declaration_location) << 
                "Could not cast " << *this << " to " << *target_type;
            return nullopt;

        case ExpressionType::missing_arg:
            type = target_type;
            return this;

        case ExpressionType::known_value_reference:
            Log::compiler_error(location) << "Casts on known value references not implemented";
            assert(false && "not implemented");
            // copy and cast

        case ExpressionType::partial_call:
        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_binop_call_both:
            Log::debug(type_declaration_location) << 
                "Expression " << *this << " in not castable due to being a partial call";
            return nullopt;

        case ExpressionType::reference:
        case ExpressionType::identifier:
        case ExpressionType::layer2_expression:
            return wrap_in_runtime_cast(state, target_type, type_declaration_location);

        case ExpressionType::partially_applied_minus:
            if (target_type->arity() != 0) {
                Log::debug(type_declaration_location) 
                    << "Expression " << *this << " in not castable due to being a partial call";
                return nullopt;
            }

            if (!convert_to_unary_minus_call(state, *this)) {
                Log::error(type_declaration_location) << "Converting to unary minus call failed";
                return nullopt;
            }
            // intentional fall-through
        case ExpressionType::call:
            if (!is_partial_call(*this))
                return wrap_in_runtime_cast(state, target_type, type_declaration_location);

            Log::debug(type_declaration_location) << "Expression " << *this <<
                " in not castable due to being a partial call";
            return nullopt;

        // case ExpressionType::lambda:
        case ExpressionType::ternary_expression:
            Log::compiler_error(location) << "Casts on lambdas and ternary expressions not implemented";
            assert(false && "not implemented");
    }
}

optional<Expression*> Expression::cast_to(CompilationState& state, const Type* target_type) {
    return this->cast_to(state, target_type, this->location);
}

optional<Expression*> Expression::wrap_in_runtime_cast(CompilationState& state, const Type* target_type, 
    const SourceLocation& type_declaration_location) {
    
    assert(is_castable_expression(*this) && "wrap_in_runtime_cast called on not a castable expression");

    auto runtime_cast = find_external_runtime_cast(*state.builtins_, type, target_type);

    if (!runtime_cast) {
        Log::debug(type_declaration_location) << "Could not cast " << *this << " to " << *target_type;
        return nullopt;
    }

    optional<Expression*> wrapper = create_call(state, *runtime_cast, {this},type_declaration_location);

    if (!wrapper) {
        Log::debug(type_declaration_location) << "Could not create cast wrapper for " << *this << 
            " to type " << *target_type;
        return nullopt;
    }

    return wrapper;
}

} // namespace Maps