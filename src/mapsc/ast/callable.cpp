#include "callable.hh"

#include <cassert>

#include "common/std_visit_helper.hh"

#include "mapsc/logging.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/types/function_type.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/builtin.hh"

using Maps::GlobalLogger::log_error;

namespace Maps {

Operator Operator::create_binary(const std::string& name, CallableBody body, Precedence precedence, 
    Associativity associativity, SourceLocation location) {

    return Operator{name, body, OperatorProps::Binary(precedence, associativity), location};
}


Callable Callable::testing_callable(const Type* type) {
    Callable callable{"DUMMY_CALLABLE", std::monostate{}, TEST_SOURCE_LOCATION};
    callable.set_type(*type);
    return callable;
}

Callable::Callable(const std::string& name, CallableBody body, const Type& type, SourceLocation location)
:Callable(name, body, location) {
    assert(!std::holds_alternative<Expression*>(body) &&
        "Tried to initialize expression-bodied callable with a type, type should be set on the expression");
    
    set_type(type);
}

Callable::Callable(const std::string& name, CallableBody body, SourceLocation location)
:body(body), name(name), location(location) {}

Callable::Callable(CallableBody body, SourceLocation location)
:body(body), name("anonymous callable"), location(location) {}

const Type* Callable::get_type() const {
    switch (body.index()) {
        case 0: // std::monostate
            return type_ ? *type_ : &Hole;
        
        case 1: // expression
            return std::get<Expression*>(body)->type;

        case 2: { // statement
            Statement* statement = std::get<Statement*>(body);

            if (statement->statement_type == StatementType::expression_statement) {
                return std::get<Expression*>(statement->value)->type;
            } 

            return type_ ? *type_ : &Hole;
        }
        case 3: // Builtin
            return std::get<Builtin*>(body)->type;

        default:
            assert(false && "unhandled CallableBody in CallableBody::get_type");
            return &Hole;
    }
}

// !!! this feels pretty sus, manipulating state with way too many layers of indirection
void Callable::set_type(const Type& type) {
    switch (body.index()) {
        case 0: // uninitialized
            type_ = std::make_optional<const Type*>(&type);
            return;
        
        case 1: // expression
            std::get<Expression*>(body)->type = &type;
            return;

        case 2: { // statement
            Statement* statement = std::get<Statement*>(body);

            if (statement->statement_type == StatementType::expression_statement) {
                std::get<Expression*>(statement->value)->type = &type;
                return;
            } 

            type_ = std::make_optional<const Type *>(&type);
            return;
        }
        case 3: // Cannot set type of a builtin
            assert(false && "tried to set_type of a builtin callable");
            return;

        default:
            assert(false && "unhandled CallableBody in CallableBody::set_type");
    }
}

std::optional<const Type*> Callable::get_declared_type() const {
    if (Expression* const* expression = std::get_if<Expression*>(&body)) {
        return (*expression)->declared_type;

    } else if (Statement* const* statement = std::get_if<Statement*>(&body)) {
        if ((*statement)->statement_type == StatementType::expression_statement)
            return std::get<Expression*>((*statement)->value)->declared_type;

        return declared_type;

    } else if (Builtin* const* builtin = std::get_if<Builtin*>(&body)) {
        return (*builtin)->type;
    }

    assert(false && "unhandled callable type in Callable::get_declared_type");
    return std::nullopt;
}

bool Callable::set_declared_type(const Type& type) {
    if (Expression* const* expression = std::get_if<Expression*>(&body)) {
        (*expression)->declared_type = &type;
        return true;

    } else if (Statement* const* statement = std::get_if<Statement*>(&body)) {
        if ((*statement)->statement_type == StatementType::expression_statement) {
            auto expression = std::get<Expression*>((*statement)->value);
            expression->declared_type = &type;
            return true;
        }

        declared_type = &type;
        return true;

    } else if (Builtin* const* builtin = std::get_if<Builtin*>(&body)) {
        log_error("tried to set the declared type on a builtin");
        assert(false && "tried to set the declared type on a builtin");
        return false;
    }

    return false;
}

bool Callable::is_undefined() const {
    return std::holds_alternative<std::monostate>(body);
}

bool Callable::is_operator() const {
    return false;
}

bool Callable::is_binary_operator() const {
    return false;
}

bool Callable::is_unary_operator() const {
    return false;
}

bool Operator::is_operator() const {
    return true;
}

bool Operator::is_binary_operator() const {
    return operator_props_.is_binary();
}

bool Operator::is_unary_operator() const {
    return operator_props_.is_unary();
}

Precedence Operator::get_precedence() {
    return operator_props_.precedence;
}


} // namespace Maps