#include "callable.hh"

#include <variant>
#include <cassert>
#include <string>

#include "common/std_visit_helper.hh"

#include "mapsc/logging.hh"

#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/expression.hh"

using std::optional, std::nullopt;


namespace Maps {

bool Callable::is_empty() const {
    return std::visit(overloaded {
        [](External) { return false; },
        [](Undefined) { return true; },
        [](const Expression*) { return false; },
        [](const Statement* statement) { return statement->is_empty(); }
    }, const_body());
}

RT_Callable RT_Callable::testing_callable(const Type* type) {
    return RT_Callable{"DUMMY_CALLABLE", External{}, *type, TEST_SOURCE_LOCATION};
}

RT_Callable::RT_Callable(std::string_view name, CallableBody body, const Type& type, 
    SourceLocation location)
: name_(name), body_(body), location_(location), type_(&type) {
    assert((!std::holds_alternative<Expression*>(body)  || 
            type == Hole                                ||
            type == *std::get<Expression*>(body)->type) &&
            "Tried to initialize expression-bodied callable with a type, \
type should be set on the expression");
}

RT_Callable::RT_Callable(std::string_view name, CallableBody body, SourceLocation location)
:RT_Callable(name, body, Hole, location) {}

RT_Callable::RT_Callable(CallableBody body, SourceLocation location)
:name_("anonymous callable"), body_(body), location_(location) {}

RT_Callable::RT_Callable(CallableBody body, const Type& type, SourceLocation location)
:name_("anonymous callable"), body_(body), location_(location), type_(&type) {}

const_CallableBody RT_Callable::const_body() const {
    return std::visit(overloaded {
        [](External external) { return const_CallableBody{external}; },
        [](Undefined undefined) { return const_CallableBody{undefined}; },
        [](Statement* statement) { return const_CallableBody{statement}; },
        [](Expression* expression) { return const_CallableBody{expression}; }
    }, body_);
}

// TODO: change to std::visitor
const Type* RT_Callable::get_type() const {
    switch (body_.index()) {
        case 0: // Undefined
            return type_ ? *type_ : &Hole;
        
        case 1: // expression
            return std::get<Expression*>(body_)->type;

        case 2: { // statement
            Statement* statement = std::get<Statement*>(body_);

            if (statement->statement_type == StatementType::expression_statement) {
                return std::get<Expression*>(statement->value)->type;
            } 

            return type_ ? *type_ : &Hole;
        }
        case 3: // External
            return *type_; 
        default: 
            assert(false && "unhandled CallableBody in CallableBody::get_type");
            return &Hole;
    }
}

// !!! this feels pretty sus, manipulating state with way too many layers of indirection
void RT_Callable::set_type(const Type& type) {
    switch (body_.index()) {
        case 0: // uninitialized
            type_ = std::make_optional<const Type*>(&type);
            return;
        
        case 1: // expression
            std::get<Expression*>(body_)->type = &type;
            return;

        case 2: { // statement
            Statement* statement = std::get<Statement*>(body_);

            if (statement->statement_type == StatementType::expression_statement) {
                std::get<Expression*>(statement->value)->type = &type;
                return;
            } 

            type_ = std::make_optional<const Type *>(&type);
            return;
        }
        case 3: // Cannot set type of an external
            assert(false && "tried to set_type of a builtin callable");
            return;

        default:
            assert(false && "unhandled CallableBody in CallableBody::set_type");
    }
}

std::optional<const Type*> RT_Callable::get_declared_type() const {
    if (Expression* const* expression = std::get_if<Expression*>(&body_)) {
        return (*expression)->declared_type;

    } else if (Statement* const* statement = std::get_if<Statement*>(&body_)) {
        if ((*statement)->statement_type == StatementType::expression_statement)
            return std::get<Expression*>((*statement)->value)->declared_type;

        return declared_type_;

    } else if (std::holds_alternative<External>(body_)) {
        return type_;
    }

    assert(false && "unhandled callable type in Callable::get_declared_type");
    return std::nullopt;
}

bool RT_Callable::set_declared_type(const Type& type) {
    if (Expression* const* expression = std::get_if<Expression*>(&body_)) {
        (*expression)->declared_type = &type;
        return true;

    } else if (Statement* const* statement = std::get_if<Statement*>(&body_)) {
        if ((*statement)->statement_type == StatementType::expression_statement) {
            auto expression = std::get<Expression*>((*statement)->value);
            expression->declared_type = &type;
            return true;
        }

        declared_type_ = &type;
        return true;

    } else if (std::holds_alternative<External>(body_)) {
        LogNoContext::compiler_error("tried to set the declared type on an external", location_);
        assert(false && "tried to set the declared type on a builtin");
        return false;
    }

    return false;
}

} // namespace Maps