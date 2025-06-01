#include "definition_body.hh"

#include "mapsc/compilation_state.hh"
#include "mapsc/ast/ast_store.hh"

namespace Maps {

DefinitionBody::DefinitionBody(DefinitionHeader* header, LetDefinitionValue value)
:header_(header), value_(value) {}

void DefinitionBody::set_type(const Type* type) {
    this->header_->type_ = type;

    std::visit(overloaded {
        [](Error) {},
        [](Undefined) {},
        [&type](Expression* expression) { expression->type = type; },
        [&type](Statement* statement) {
            if (statement->statement_type == StatementType::expression_statement)
                std::get<Expression*>(statement->value)->type = type;
        },
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

} // namespace Maps