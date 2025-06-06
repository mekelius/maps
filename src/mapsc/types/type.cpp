#include "type.hh"

#include <cassert>
#include <optional>

#include "mapsc/source_location.hh"
#include "mapsc/logging.hh"

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/expression_properties.hh"

using std::optional, std::nullopt;

namespace Maps {

using Log = LogInContext<LogContext::type_casts>;

bool Type::cast_to(const Type* target_type, Expression& expression) const {
    assert(*expression.type == *this && 
        "Type::cast_to called with an expression of a type other than *this");

    if (*expression.type == *target_type) {
        Log::debug_extra(expression.location) << "Already same type" << Endl;
        return true;
    }

    if (!is_castable_expression(expression)) {
        Log::error(expression.location) << expression << ", is not castable" << Endl;
        return false;
    }
    
    if (is_constant_value(expression))
        return cast_to_(target_type, expression);

    Log::debug(expression.location) << expression << " is not constant, refusing to cast" << Endl;
    return false;
}

} // namespace Maps