#include "definition.hh"

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

using Log = LogNoContext;

bool Definition::is_empty() const {
    return std::visit(overloaded {
        [](Error) { return true; },
        [](External) { return false; },
        [](Undefined) { return true; },
        [](const Expression*) { return false; },
        [](const Statement* statement) { return statement->is_empty(); }
    }, const_body());
}

RT_Definition RT_Definition::testing_definition(const Type* type) {
    return RT_Definition{"DUMMY_DEFINITION", External{}, *type, TEST_SOURCE_LOCATION};
}

RT_Definition::RT_Definition(std::string_view name, DefinitionBody body, const Type& type, 
    SourceLocation location)
: name_(name), body_(body), location_(location), type_(&type) {
    assert((!std::holds_alternative<Expression*>(body)  || 
            type == Hole                                ||
            type == *std::get<Expression*>(body)->type) &&
            "Tried to initialize expression-bodied definition with a type, \
type should be set on the expression");
}

RT_Definition::RT_Definition(std::string_view name, DefinitionBody body, SourceLocation location)
:RT_Definition(name, body, Hole, location) {}

RT_Definition::RT_Definition(DefinitionBody body, SourceLocation location)
:name_("anonymous definition"), body_(body), location_(location) {}

RT_Definition::RT_Definition(DefinitionBody body, const Type& type, SourceLocation location)
:name_("anonymous definition"), body_(body), location_(location), type_(&type) {}


const_DefinitionBody RT_Definition::const_body() const {
    return std::visit([](auto body) { return const_DefinitionBody{body}; }, body_);
}

// TODO: change to std::visitor
const Type* RT_Definition::get_type() const {
    return std::visit(overloaded {
        [](Error error) -> const Type* { (void) error; return &Absurd;},
        [this](Undefined undefined) -> const Type* { (void) undefined; return type_ ? *type_ : &Hole; },
        [this](External external) -> const Type* { (void) external; return *type_; },

        [](Expression* expression) -> const Type* { return expression->type; },

        [this](Statement* statement) -> const Type* {
            if (statement->statement_type == StatementType::expression_statement)
                return std::get<Expression*>(statement->value)->type;

            return type_ ? *type_ : &Hole;
        },
    }, body_);
}

// !!! this feels pretty sus, manipulating state with way too many layers of indirection
void RT_Definition::set_type(const Type& type) {
    std::visit(overloaded {
        [](Error error) { (void) error; },
        [this](External external) { 
            (void) external; 
            Log::compiler_error("Attempting to set the type of an external", this->location()); 
        },

        [this, &type](Undefined undefined) { 
            type_ = std::make_optional<const Type*>(&type);
            Log::compiler_error("Attempting to set the type of an undefined", this->location()); 
        },

        [&type](Expression* expression) { expression->type = &type; },

        [this, &type](Statement* statement) {
            if (statement->statement_type == StatementType::expression_statement) {
                std::get<Expression*>(statement->value)->type = &type;
                return;
            } 

            type_ = std::make_optional<const Type *>(&type);
        },
    }, body_);
}

std::optional<const Type*> RT_Definition::get_declared_type() const {
    if (Expression* const* expression = std::get_if<Expression*>(&body_)) {
        return (*expression)->declared_type;

    } else if (Statement* const* statement = std::get_if<Statement*>(&body_)) {
        if ((*statement)->statement_type == StatementType::expression_statement)
            return std::get<Expression*>((*statement)->value)->declared_type;

        return declared_type_;

    } else if (std::holds_alternative<External>(body_)) {
        return type_;
    }

    assert(false && "unhandled definition type in Definition::get_declared_type");
    return std::nullopt;
}

bool RT_Definition::set_declared_type(const Type& type) {
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