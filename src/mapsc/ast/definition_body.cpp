#include "definition_body.hh"

#include <variant>

#include "mapsc/compilation_state.hh"
#include "mapsc/ast/ast_store.hh"

namespace Maps {

DefinitionBody::DefinitionBody(DefinitionHeader* header, LetDefinitionValue value)
:header_(header) {
    set_value(value);
}

LetDefinitionValue DefinitionBody::get_value() const {
    return value_;
}

void DefinitionBody::set_value(LetDefinitionValue value) {
    value_ = value;

    std::visit(overloaded {
        [this](Expression* expression) {
            set_type(expression->type);
        },
        [this](Statement* statement) {
            switch (statement->statement_type) {
                case StatementType::return_:
                case StatementType::expression_statement:
                    set_type(statement->get_value<Expression*>()->type);
                    return;
                default:
                    return;
            }
        },
        [](auto) { return; }
    }, value);
}

void DefinitionBody::set_type(const Type* type) {
    this->header_->type_ = type;

    std::visit(overloaded {
        [](Error) {},
        [](Undefined) {},
        [&type](Expression* expression) { expression->type = type; },
        [&type](Statement* statement) { statement->type = type; },
        [](BuiltinValue value) {
            assert(false && "trying to set type on a builtin");
        }
    }, value_);
}

std::optional<const Type*> DefinitionBody::get_declared_type() const {
    if (Expression* const* expression = std::get_if<Expression*>(&value_)) {
        return (*expression)->declared_type;

    } else if (Statement* const* statement = std::get_if<Statement*>(&value_)) {
        if ((*statement)->statement_type == StatementType::expression_statement)
            return std::get<Expression*>((*statement)->value)->declared_type;

        return declared_type_;
    }

    assert(false && "unhandled definition type in Definition::get_declared_type");
    return std::nullopt;
}

bool DefinitionBody::set_declared_type(const Type* type) {
    if (Expression* const* expression = std::get_if<Expression*>(&value_)) {
        (*expression)->declared_type = type;
        return true;

    } else if (Statement* const* statement = std::get_if<Statement*>(&value_)) {
        if ((*statement)->statement_type == StatementType::expression_statement) {
            auto expression = std::get<Expression*>((*statement)->value);
            expression->declared_type = type;
            return true;
        }

        declared_type_ = type;
        return true;
    }

    return false;
}

LogStream::InnerStream& DefinitionBody::log_self_to(LogStream::InnerStream& ostream) const {
    std::visit(overloaded {
        [&ostream](Undefined)   { ostream << "(undefined)";  },
        [&ostream](Expression*) { ostream << "(expression-valued)"; },
        [&ostream](Statement*)  { ostream << "(statement-valued)";  },
        [&ostream](Error)       { ostream << "(ERROR-valued)";      },
    }, value_);

    return ostream << " definition body of " << header_->log_representation();
}

} // namespace Maps