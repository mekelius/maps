#include "simplify.hh"

#include <variant>
#include <vector>

#include "common/std_visit_helper.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/types/type.hh"
#include "mapsc/types/function_type.hh"

#include "mapsc/procedures/inline.hh"

namespace Maps {

// TODO: delete the simplified nodes properly
bool attempt_simplify(RT_Definition& definition) {
    return std::visit(overloaded {
        [](Undefined) { return true; },
        [](External) { return true; },

        [&definition](Statement* statement) {
            switch (statement->statement_type) {                
                // expression statements get replaced by their expressions
                case StatementType::expression_statement:
                    // TODO: check type
                    definition.body() = std::get<Expression*>(statement->value);
                    statement->statement_type = StatementType::deleted;
                    return attempt_simplify(definition);
                    
                case StatementType::block: {
                    // TODO: check type
                    auto block = &std::get<Block>(statement->value);

                    if (block->size() == 0) {
                        definition.body() = Undefined{};
                        statement->statement_type = StatementType::deleted;
                        return true;
                    }

                    if (block->size() == 1) {
                        *statement = *block->back();
                        // definition SEGFAULTS! ???:
                        // block->back()->statement_type = StatementType::deleted;
                        return attempt_simplify(definition);
                    }

                    return false;
                }

                // pure functions and non-function return statements get converted to expressions
                case StatementType::return_: {
                    auto type = definition.get_type();
                    if (!type->is_function()) {
                        statement->statement_type = StatementType::deleted;
                        definition.body() = std::get<Expression*>(statement->value);
                        return attempt_simplify(definition);
                    }

                    if (type->is_pure() && type->arity() == 0) {
                        definition.body() = std::get<Expression*>(statement->value);
                        statement->statement_type = StatementType::deleted;
                        return attempt_simplify(definition);
                    }

                    return false;
                }

                case StatementType::empty:
                    statement->statement_type = StatementType::deleted;
                    definition.body() = Undefined{};
                    return true;

                default:
                    return false;
            }
        },

        [&definition](Expression* expression) {
            switch (expression->expression_type) {
                case ExpressionType::call: {
                    auto type = expression->type;
                    if (!type->is_function())
                        return inline_call(*expression, definition);

                    if (type->is_pure() && type->arity() == 0)
                        return inline_call(*expression, definition);

                    return false;
                }

                default:
                    return false;
            }
        },
    }, definition.body());
}

} // namespace Maps