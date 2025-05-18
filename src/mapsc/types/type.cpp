#include "type.hh"

#include <cassert>
#include <optional>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/ast/expression.hh"


using std::optional, std::nullopt;

namespace Maps {

using Log = LogNoContext;

bool Type::cast_to(const Type* target_type, Expression& expression) const {
    assert(*expression.type == *this && 
        "Type::cast_to called with an expression of a type other than *this");

    if (*expression.type == *target_type)
        return true;

    if (!expression.is_castable_expression()) {
        Log::error("expression " + expression.log_message_string() + ", is not castable", 
            expression.location);
        return false;
    }
    
    if (expression.is_constant_value())
        return cast_to_(target_type, expression);

    return false;
}

// bool SimpleType::concretize(Expression& expression) const {
//     if (*expression.type != *this) {
//         log_error("Type::concretize called with an expression of another type", expression.location);
//         assert(false && "Type::concretize called with an expression of another type");
//         return false;
//     }

//     return concretize_(expression);
// }

} // namespace Maps