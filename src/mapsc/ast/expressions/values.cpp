#include "../expression.hh"

#include <variant>
#include <optional>

using std::optional, std::nullopt;

#include "common/std_visit_helper.hh"
#include "mapsc/logging.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

namespace Maps {

bool Expression::is_constant_value() const {
    switch (expression_type) {
        case ExpressionType::known_value:
        case ExpressionType::numeric_literal:
        case ExpressionType::string_literal:
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

Expression* Expression::string_literal(AST_Store& store, const std::string& value, 
    const SourceLocation& location) {
    
    return store.allocate_expression({ExpressionType::string_literal, value, &String, location});
}

Expression* Expression::numeric_literal(AST_Store& store, const std::string& value, 
    const SourceLocation& location) {
    
    return store.allocate_expression(
        {ExpressionType::numeric_literal, value, &NumberLiteral, location});
}

Expression* Expression::known_value(AST_Store& store, KnownValue value,
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

    return store.allocate_expression({ExpressionType::known_value, unwrapped_value, type, location});
}

optional<Expression*> Expression::known_value(AST_Store& store, KnownValue value, const Type* type,
    const SourceLocation& location) {

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

    auto expression = store.allocate_expression(
        {ExpressionType::known_value, unwrapped_value, de_facto_type, location});

    if (*de_facto_type == *type)
        return expression;

    // try to cast
    Log::debug_extra("While creating known value expression: de-facto type didn't match given type, attempting to cast...", 
        location);

    if (!de_facto_type->cast_to(type, *expression)) {
        Log::error("Couldn't create a known value of type " + type->name_string() + 
            " from value:" + Expression::value_to_string(value), location);
        return nullopt;
    }

    return expression;
}

} // namespace Maps
