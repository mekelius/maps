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

KnownValue to_known_value(const BuiltinValue& value) {
    return std::visit( [](auto value) { return KnownValue{value}; }, value.value_);
}

ExpressionValue to_expression_value(const BuiltinValue& value) {
    return std::visit( [](auto value) { return ExpressionValue{value}; }, value.value_);
}

Expression* create_string_literal(AST_Store& store, const std::string& value, 
    const SourceLocation& location) {
    
    return store.allocate_expression({ExpressionType::known_value, value, &String, location});
}

Expression* create_numeric_literal(AST_Store& store, const std::string& value, 
    const SourceLocation& location) {
    
    return store.allocate_expression(
        {ExpressionType::known_value, value, &NumberLiteral, location});
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
    Log::debug_extra(location) << "While creating known value expression: " << 
        "de-facto type didn't match given type, attempting to cast..." << Endl;

    if (!expression->cast_to(state, type)) {
        Log::error(location) << "Couldn't create a known value of type " << *type << 
            " from value:" << value_to_string(value) << Endl;
        return nullopt;
    }

    Log::debug_extra(location) << "Succesfully casted to " << *type << Endl;

    return expression;
}

Expression& convert_to_known_value(Expression& expression, const BuiltinValue& value) {
    expression.expression_type = ExpressionType::known_value;
    expression.value = to_expression_value(value);
    expression.type = value.type_;

    return expression;
}

optional<KnownValue> Expression::known_value_value() const {
    assert((expression_type == ExpressionType::known_value) && 
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
            LogNoContext::compiler_error(location) << *this << " held an incorrect value type" << Endl;
            assert(false && "known_value expression didn't have a correct value type");
            return nullopt;
        }
    }, value);
}

const Type* deduce_type(KnownValue value) {
    return std::visit(overloaded{
        [](maps_Int) { return &Int; },
        [](maps_Float) { return &Float; },
        [](bool) { return &Boolean; },
        [](std::string) { return &String; },
        [](maps_MutString) { return &MutString; },
    }, value);
}

} // namespace Maps
