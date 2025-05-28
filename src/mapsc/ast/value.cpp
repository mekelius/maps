#include "value.hh"

#include <variant>
#include <optional>
#include <utility>

using std::optional, std::nullopt, std::holds_alternative;

#include "common/std_visit_helper.hh"
#include "mapsc/logging.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

namespace Maps {

bool Expression::is_constant_value() const {
    switch (expression_type) {
        case ExpressionType::numeric_literal:
        case ExpressionType::string_literal:
            assert(holds_alternative<std::string>(value) && 
                "Encountered an expression with literal expressiontype but wrong type of body");
            return true;

        case ExpressionType::known_value:
            assert((!holds_alternative<CallExpressionValue>(value)) && 
                "Encountered an expression with expressiontype known value but wrong type of body");
            return true;

        default:
            return false;   
    }
}

bool Expression::is_literal() const {
    switch (expression_type) {
        case ExpressionType::numeric_literal:
        case ExpressionType::string_literal:
            return true;

        default:
            return false;
    }
}

Expression* create_string_literal(AST_Store& store, const std::string& value, 
    const SourceLocation& location) {
    
    return store.allocate_expression({ExpressionType::string_literal, value, &String, location});
}

Expression* create_numeric_literal(AST_Store& store, const std::string& value, 
    const SourceLocation& location) {
    
    return store.allocate_expression(
        {ExpressionType::numeric_literal, value, &NumberLiteral, location});
}

Expression* create_known_value(CompilationState& state, KnownValue value,
    const SourceLocation& location) {

    auto [unwrapped_value, type] = std::visit(overloaded{
        [](maps_Int value)->std::pair<ExpressionValue, const Type*> 
            { return {value, &Int}; },
        [](maps_Float value)->std::pair<ExpressionValue, const Type*> 
            { return {value, &Float}; },
        [](bool value)->std::pair<ExpressionValue, const Type*> 
            { return {value, &Boolean}; },
        [](std::string value)->std::pair<ExpressionValue, const Type*> 
            { return {value, &String}; },
        [](maps_MutString value)->std::pair<ExpressionValue, const Type*> 
            { return {value, &MutString}; },
    }, value);

    return state.ast_store_->allocate_expression(
        {ExpressionType::known_value, unwrapped_value, type, location});
}

optional<Expression*> create_known_value(CompilationState& state, KnownValue value, 
    const Type* type, const SourceLocation& location) {

    using Log = LogInContext<LogContext::type_checks>;

    auto [unwrapped_value, de_facto_type] = std::visit(overloaded{
        [](maps_Int value)->std::pair<ExpressionValue, const Type*> 
            { return {value, &Int}; },
        [](maps_Float value)->std::pair<ExpressionValue, const Type*> 
            { return {value, &Float}; },
        [](bool value)->std::pair<ExpressionValue, const Type*> 
            { return {value, &Boolean}; },
        [](std::string value)->std::pair<ExpressionValue, const Type*> 
            { return {value, &String}; },
        [](maps_MutString value)->std::pair<ExpressionValue, const Type*> 
            { return {value, &MutString}; },
    }, value);

    auto expression = state.ast_store_->allocate_expression(
        {ExpressionType::known_value, unwrapped_value, de_facto_type, location});

    if (*de_facto_type == *type)
        return expression;

    // try to cast
    Log::debug_extra("While creating known value expression: de-facto type didn't match given type, attempting to cast...", 
        location);

    if (!expression->cast_to(state, type)) {
        Log::error("Couldn't create a known value of type " + type->name_string() + 
            " from value:" + Expression::value_to_string(value), location);
        return nullopt;
    }

    Log::debug_extra("Succesfully casted to " + type->name_string(), location);

    return expression;
}

optional<KnownValue> Expression::known_value_value() const {
    assert((expression_type == ExpressionType::known_value || 
        expression_type == ExpressionType::string_literal || 
        expression_type == ExpressionType::numeric_literal) && 
        "known_value_value called on not a known_value");

    return std::visit(overloaded {
        [](maps_Int value)->optional<KnownValue>
            { return {value}; },
        [](maps_Float value)->optional<KnownValue>
            { return {value}; },
        [](bool value)->optional<KnownValue>
            { return {value}; },
        [](std::string value)->optional<KnownValue>
            { return {value}; },
        [](maps_MutString value)->optional<KnownValue>
            { return {value}; },
        [this](auto)->optional<KnownValue> {
            assert(false && "known_value expression didn't have a correct value type");
            LogNoContext::compiler_error(log_message_string() + " held an incorrect value type", 
                location);
            return nullopt;
        }
    }, value);
}

const Type* Expression::deduce_type(KnownValue value) {
    return std::visit(overloaded{
        [](maps_Int) { return &Int; },
        [](maps_Float) { return &Float; },
        [](bool) { return &Boolean; },
        [](std::string) { return &String; },
        [](maps_MutString) { return &MutString; },
    }, value);
}

} // namespace Maps
