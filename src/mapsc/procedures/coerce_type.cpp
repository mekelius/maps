#include "coerce_type.hh"

#include <vector>

#include "type_check.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/statement.hh"

namespace Maps {

bool try_to_coerce(Expression& expression, const Type* type) {
    return expression.type->cast_to(type, expression);
}

bool handle_declared_type(Expression& expression, const Type* declared_type) {
    return try_to_coerce(expression, declared_type);
}

// the first return value signals whether a coercion was performed,
// the second one whether a return value was found
std::pair<bool, bool> coerce_block_return_type(Statement& statement, const Type* wanted_type) {
    std::vector<Statement*> block = std::get<Block>(statement.value);
    
    for (auto substatement: block) {
        switch (substatement->statement_type) {
            // TODO: handle conditional returns
            case StatementType::block: {
                auto [coerced, had_return] = coerce_block_return_type(*substatement, wanted_type);
                if (had_return)
                    return {coerced, had_return};
                    
                continue;
            }
            case StatementType::return_:
                return {
                    handle_declared_type(*std::get<Expression*>(substatement->value), wanted_type), 
                    true
                };

            default:
                continue;
        }
    }

    return {false, false};
}

// Gets the return type of a statement, trying to coerce into the wanted type if given
bool coerce_return_type(Statement& statement, const Type* wanted_type) {
    switch (statement.statement_type) {
        case StatementType::block:
            return coerce_block_return_type(statement, wanted_type).first;
        
        case StatementType::expression_statement:
        case StatementType::return_:
            return handle_declared_type(*std::get<Expression*>(statement.value), wanted_type);
        default:
            return false;
    }
}

} // namespace Maps