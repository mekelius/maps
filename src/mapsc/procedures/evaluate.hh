#ifndef __EVALUATE_HH
#define __EVALUATE_HH

#include <optional>
#include <variant>

#include "common/std_visit_helper.hh"

#include "mapsc/logging.hh"

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/value.hh"

#include "mapsc/ast/definition.hh"

namespace Maps {

inline std::optional<KnownValue> evaluate(const DefinitionBody& definition) {
    using std::nullopt;
    using Log = LogInContext<LogContext::eval>;

    return std::visit(overloaded{
        [](Expression* expression)->std::optional<KnownValue> {
            switch (expression->expression_type) {
                case ExpressionType::known_value_reference: {
                    assert(expression->reference_value()->body_ && 
                        "evaluate called on a definition without a body");
                    return evaluate(**expression->reference_value()->body_);
                }

                case ExpressionType::known_value: {
                    Log::debug_extra(expression->location) << "Evaluating " << *expression << Endl;
                    auto value = expression->known_value_value();

                    if (!value) {
                        Log::error(expression->location) << 
                            "Evaluating " << *expression << " failed" << Endl;
                        return nullopt;
                    }

                    Log::debug_extra(expression->location) << 
                        "Evaluated to " << value_to_string(*value) << Endl;
                    return value;
                }
                default:
                    Log::compiler_error(expression->location) << "Compile-time evaluating " << 
                        expression->expression_type_string() << " not implemented" << Endl;
                    return nullopt;
            }
        },
        [definition](auto)->std::optional<KnownValue> {
            Log::compiler_error(definition.location()) << 
                "Compile time evaluating definitions with body type " << 
                definition.node_type_string() << " not implemented" << Endl;
            return nullopt;
        }
    }, definition.get_value());
}

} // namespace Maps

#endif