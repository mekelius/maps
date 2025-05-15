#include "type.hh"

#include <cassert>
#include <optional>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/ast/expression.hh"

namespace Maps {

// using GlobalLogger::log_error;

// using std::optional, std::nullopt;

// bool SimpleType::cast_to(const SimpleType* target_type, Expression& expression) const {
    
//     if (*expression.type == *target_type)
//         return true;

//     if (!expression.is_castable_expression()) {
//         log_error("expression " + expression.log_message_string() + ", is not castable");
//         return false;
//     }
    
//     return cast_to_(target_type, expression);
// }

// bool SimpleType::concretize(Expression& expression) const {
//     if (*expression.type != *this) {
//         log_error("Type::concretize called with an expression of another type", expression.location);
//         assert(false && "Type::concretize called with an expression of another type");
//         return false;
//     }

//     return concretize_(expression);
// }

} // namespace Maps