#ifndef __EVALUATE_HH
#define __EVALUATE_HH

#include <optional>
#include <variant>

#include "common/std_visit_helper.hh"

#include "mapsc/logging.hh"

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/definition.hh"

namespace Maps {

inline std::optional<KnownValue> evaluate(const Definition* definition) {
    using std::nullopt;
    using Log = LogInContext<LogContext::eval>;

    return std::visit(overloaded{
        [](Undefined)->std::optional<KnownValue> { return nullopt; },
        [definition](External)->std::optional<KnownValue> { 
            Log::compiler_error("Compile time evaluating externals not implemented", 
                definition->location());
            return nullopt;
        },
        [](const Expression* expression)->std::optional<KnownValue> {
            switch (expression->expression_type) {
                case ExpressionType::known_value_reference:
                    return evaluate(expression->reference_value());

                case ExpressionType::known_value: {
                    Log::debug_extra("Evaluating " + expression->log_message_string(), 
                        expression->location);
                    auto value = expression->known_value_value();

                    if (!value) {
                        Log::error("Evaluating " + expression->log_message_string() + " failed", 
                            expression->location);
                        return nullopt;
                    }

                    Log::debug_extra("Evaluated to " + Expression::value_to_string(*value), 
                        expression->location);
                    return value;
                }
                default:
                    Log::compiler_error("Compile-time evaluating " + 
                        expression->expression_type_string() + 
                        " not implemented", expression->location);
                    return nullopt;
            }
        },
        [definition](auto)->std::optional<KnownValue> {
            Log::compiler_error("Compile time evaluating definitions with body type " + 
                definition->body_type_string() + " not implemented", definition->location());
            return nullopt;
        }
    }, definition->const_body());
}

} // namespace Maps

#endif