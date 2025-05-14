#include "type.hh"

#include <cassert>
#include <optional>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/ast/expression.hh"

namespace Maps {

using GlobalLogger::log_error;

using std::optional, std::nullopt;

// todo: memoize this on the type
std::string Type::to_string() const {
    return static_cast<std::string>(name());
}

bool Type::cast_to(const Type* target_type, Expression& expression) const {
    
    if (*expression.type == *target_type)
        return true;

    if (!expression.is_castable_expression()) {
        log_error("expression " + expression.log_message_string() + ", is not castable");
        return false;
    }
    
    return cast_to_(target_type, expression);
}

bool Type::concretize(Expression& expression) const {
    if (*expression.type != *this) {
        log_error("Type::concretize called with an expression of another type", expression.location);
        assert(false && "Type::concretize called with an expression of another type");
        return false;
    }

    return concretize_(expression);
}

// cast_to above does some safety checks that shouldn't be overridden
bool Type::cast_to_(const Type* type, Expression& expression) const {
    return (*cast_function_)(type, expression);
}

// concretize above does some safety checks that shouldn't be overridden
bool Type::concretize_(Expression& expression) const {
    return (*concretize_function_)(expression);
}

} // namespace Maps