#include "simplify.hh"

#include <variant>

#include "common/std_visit_helper.hh"

#include "mapsc/ast/statement.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/procedures/inline.hh"

namespace Maps {

// TODO: delete the simplified nodes properly
bool attempt_simplify(Callable& callable) {
    return std::visit(overloaded {
        [](Undefined) { return true; },
        [](External) { return true; },

        [&callable](Statement* statement) {
            switch (statement->statement_type) {                
                // expression statements get replaced by their expressions
                case StatementType::expression_statement:
                    // TODO: check type
                    callable.body_ = std::get<Expression*>(statement->value);
                    statement->statement_type = StatementType::deleted;
                    return attempt_simplify(callable);
                    
                case StatementType::block: {
                    // TODO: check type
                    auto block = &std::get<Block>(statement->value);

                    if (block->size() == 0) {
                        callable.body_ = Undefined{};
                        statement->statement_type = StatementType::deleted;
                        return true;
                    }

                    if (block->size() == 1) {
                        *statement = *block->back();
                        // callable SEGFAULTS! ???:
                        // block->back()->statement_type = StatementType::deleted;
                        return attempt_simplify(callable);
                    }

                    return false;
                }

                // pure functions and non-function return statements get converted to expressions
                case StatementType::return_: {
                    auto type = callable.get_type();
                    if (!type->is_function()) {
                        statement->statement_type = StatementType::deleted;
                        callable.body_ = std::get<Expression*>(statement->value);
                        return attempt_simplify(callable);
                    }

                    auto function_type = dynamic_cast<const FunctionType*>(type);
                    if (function_type->is_pure_ && function_type->arity() == 0) {
                        callable.body_ = std::get<Expression*>(statement->value);
                        statement->statement_type = StatementType::deleted;
                        return attempt_simplify(callable);
                    }

                    return false;
                }

                case StatementType::empty:
                    statement->statement_type = StatementType::deleted;
                    callable.body_ = Undefined{};
                    return true;

                default:
                    return false;
            }
        },

        [&callable](Expression* expression) {
            switch (expression->expression_type) {
                case ExpressionType::call: {
                    auto type = expression->type;
                    if (!type->is_function())
                        return inline_call(*expression, callable);

                    auto function_type = dynamic_cast<const FunctionType*>(type);
                    if (function_type->is_pure_ && function_type->arity() == 0)
                        return inline_call(*expression, callable);

                    return false;
                }

                default:
                    return false;
            }
        },
    }, callable.body_);
}

} // namespace Maps