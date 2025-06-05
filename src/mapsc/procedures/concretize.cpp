#include "concretize.hh"

#include <cassert>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include "mapsc/source_location.hh"
#include "mapsc/logging.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/ast/expression_properties.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/call_expression.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/types/type.hh"
#include "mapsc/types/function_type.hh"

#include "mapsc/procedures/inline.hh"

using std::get, std::get_if, std::optional, std::nullopt;


namespace Maps {

using Log = LogInContext<LogContext::concretize>;


namespace {

optional<const Type*> deduce_return_type(TypeStore& types, 
    const std::vector<const Type*> potential_return_types) {
    
    if (potential_return_types.size() == 1)
        return potential_return_types.back();

    if (potential_return_types.empty())
        return nullopt;

    // For now we just check if they are all the same
    const Type* return_type = potential_return_types.back();
    for (auto type: potential_return_types)
        if (*type != *return_type)
            return nullopt;

    return return_type;
}

bool concretize_return(CompilationState& state, std::vector<const Type*>& potential_return_types, 
    Statement& statement) {

    auto expression = statement.get_value<Expression*>();
    if (!concretize(state, *expression))
        return false;
    potential_return_types.push_back(expression->type);
    return true;
}

bool concretize_block(CompilationState& state, std::vector<const Type*>& potential_return_types, 
    Statement& statement) {

    for (auto substatement: statement.get_value<Block>()) {
        if (!concretize(state, potential_return_types, *substatement))
            return false;

        // we can short circuit on unconditional return
        if (substatement->statement_type == StatementType::return_)
            return true;
    }

    return true;
}

bool concretize_conditional(CompilationState& state, std::vector<const Type*>& potential_return_types, 
    Statement& statement) {

    auto [condition, body, else_branch] = statement.get_value<ConditionalValue>();
    if (!concretize(state, potential_return_types, *body))
        return false;
    
    if (!else_branch)
        return true;

    return concretize(state, potential_return_types, **else_branch);
}

bool concretize_loop(CompilationState& state, std::vector<const Type*>& potential_return_types, 
    Statement& statement) {

    return concretize(state, potential_return_types, *statement.get_value<LoopStatementValue>().body);
}

bool concretize_call(CompilationState& state, Expression& call) {
    auto [callee, args] = call.call_value();

    Log::debug_extra(call.location) << "Concretizing a call to " << *callee << Endl;

    // attempt inline first
    Log::debug_extra(call.location) << "Attempting to inline " << call << Endl;

    if (callee->body_)
        if (inline_call(call, **callee->body_))
            return concretize(state, call);

    Log::debug_extra(call.location) << "Could not inline, attempting to cast arguments" << Endl;

    auto callee_type = dynamic_cast<const FunctionType*>(callee->get_type());

    // if it's not a function, it should have been inlinable?
    if (!callee_type) {
        Log::warning(call.location) << 
            "Concretizing " << call << " failed, was not a function and could not inline" << Endl;
        return false;
    }

    if (call.declared_type) {
        Log::debug_extra(call.location) << "Call has a declared type" << Endl;

        if (**call.declared_type != *callee_type) {
            assert(false && "mismatching declared type not implemented in concretize call");
        }

        if (!callee->get_type()->is_function()) {
            if (!args.empty())
                return false;
    
            // ???
        }
    }

    // handle args
    assert(args.size() <= callee_type->arity() && "call expression has too many args");
    assert(args.size() == callee_type->arity() && "partial calls not implemented in concretize_call");

    for (int i = 0; auto param_type: callee_type->param_types()) {
        auto arg = args.at(i++);

        if (*arg->type == *param_type)
            continue;

        if (is_constant_value(*arg)) { 
            Log::debug_extra(call.location) << 
                "Substituting constant argument: \"" << *arg << "\". Attempting to cast from " << 
                *arg->type << " into " << *param_type << Endl;
            
            if (!arg->type->cast_to(param_type, *arg)) {
                Log::error(arg->location) << "No" << Endl;
                return false;
            }
        }
        if (!concretize(state, *arg))
            return false;

        if (*arg->type != *param_type) {
            Log::error(arg->location) << 
                *arg << " does not match parameter type: " << *param_type << Endl;
            return false;
        }
    }

    // handle return type
    auto return_type = callee_type->return_type();

    if (return_type->is_concrete())
        return true;
    
    assert(false && "concretizing return values not implemented");
    // create a call to cast function here

    return false;
}

bool concretize_reference(Expression& value) {
    if (!value.declared_type)
        return substitute_value_reference(value);

    if (!value.type->concretize(value))
        return false;

    return true;
}

bool concretize_value(Expression& value) {
    return(value.type->concretize(value));
}

} // namespace


bool concretize(CompilationState& state, DefinitionBody& definition) {
    return std::visit(overloaded{
        [&definition, &state](Expression* expression) {
            Log::debug_extra(definition.location()) << 
                "Concretizing definition body of " << definition << Endl;
            if (!concretize(state, *expression)) {
                Log::error(definition.location()) << "Concretizing definition body failed" << Endl;
                return false;
            }

            definition.set_type(expression->type);
            return true;
        },
        [&definition, &state](Statement* statement) {
            Log::debug_extra(definition.location()) << 
                "Concretizing definition body of " << definition << Endl;

            // Need to figure out the return type
            std::vector<const Type*> potential_return_types {};
            if (!concretize(state, potential_return_types, *statement))
                return false;

            optional<const Type*> deduced_return_type = 
                deduce_return_type(*state.types_, potential_return_types);
            if (!deduced_return_type) {
                Log::error(definition.location()) << 
                    "No viable return type for " << definition << Endl;
                return false;
            }
            Log::debug_extra(definition.location()) << 
                "Deduced type for " << definition << "(" << **deduced_return_type << ")" << Endl;

            auto function_type = state.types_->get_function_type(*deduced_return_type, {}, false);
            definition.set_type(function_type);
            return true;
        },
        [definition](auto) { 
            Log::info(definition.location()) << 
                "Not concretizing " << definition << ", unhandled definition body type" << Endl;
            return true; 
        }
    }, definition.body());
}

bool concretize(CompilationState& state, Expression& expression) {
    Log::debug_extra(expression.location) << "Concretizing " << expression << Endl;

    switch (expression.expression_type) {
        case ExpressionType::call:
            return concretize_call(state, expression);

        case ExpressionType::known_value_reference:
            if (!substitute_value_reference(expression)) {
                Log::compiler_error(expression.location) << 
                    "Substituting known value reference failed" << Endl;
                return false;
            }
            assert(expression.expression_type == ExpressionType::known_value &&
                "substitute value reference on known value reference didn't result in a known value");
            return concretize_value(expression);

        case ExpressionType::known_value:
            return concretize_value(expression);

        case ExpressionType::reference:
            return concretize_reference(expression);

        case ExpressionType::partially_applied_minus:
            if (!convert_to_unary_minus_call(state, expression))
                return false;
            return concretize_call(state, expression);

        default:
            Log::error(expression.location) <<
                "Concretizer encountered an expression that was not a value or a call" << Endl;
            return false;
    }
}

bool concretize(CompilationState& state, std::vector<const Type*>& potential_return_types, 
    Statement& statement) {

    switch (statement.statement_type) {
        case StatementType::user_error:
        case StatementType::compiler_error:
        case StatementType::deleted:
        case StatementType::empty:
        case StatementType::assignment:
            return false;
            
        // single expression statements get an implicit return
        case StatementType::expression_statement: 
            return concretize(state, *statement.get_value<Expression*>());

        case StatementType::return_:
            return concretize_return(state, potential_return_types, statement);

        case StatementType::block:
            return concretize_block(state, potential_return_types, statement);

        case StatementType::switch_s:
            assert(false && "not implemented");

        case StatementType::conditional:
            return concretize_conditional(state, potential_return_types, statement);

        case StatementType::loop:
            return concretize_loop(state, potential_return_types, statement);

        default:
            Log::error(statement.location) << 
                "Concretizer encountered an unhandled statement type:" << 
                statement.statement_type_string() << Endl;
            return false;
    }
}

} // namespace Maps
