#include "ast_node.hh"

#include <cassert>

#include "../logging.hh"
#include "words.hh"

using Logging::log_error;

namespace Maps {

// ----- EXPRESSION -----

bool Expression::is_partial_call() const {
    if (expression_type != ExpressionType::call)
        return false;

    auto [callee, args] = std::get<CallExpressionValue>(value);

    if (args.size() < callee->get_type()->arity())
        return true;

    for (auto arg: args) {
        if (arg->expression_type == ExpressionType::missing_arg)
            return true;
    }

    return false;
}

bool Expression::is_reduced_value() const {
    switch (expression_type) {
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
        case ExpressionType::reference:
        case ExpressionType::call:
            return true;

        default:
            return false;
    }
}

// TODO: clean this up
const std::string& Expression::string_value() const {
    if (std::holds_alternative<Callable*>(value)) {
        // !!! this will cause crashes when lambdas come in
        return std::get<Callable*>(value)->name;
    }
    return std::get<std::string>(value);
}

bool Expression::is_literal() const {
    switch (expression_type) {
        case ExpressionType::numeric_literal:
        case ExpressionType::string_literal:
            return true;

        default:
            return false;
    }
}

bool Expression::is_illegal() const {
    switch (expression_type) {
        case ExpressionType::deleted:
        case ExpressionType::not_implemented:
        case ExpressionType::syntax_error:
            return true;

        default:
            return false;
    }
}

bool Expression::is_reference() const {
    switch (expression_type) {
        case ExpressionType::reference:
        case ExpressionType::type_reference:
        case ExpressionType::operator_reference:
        case ExpressionType::type_operator_reference:
        case ExpressionType::type_constructor_reference:
            return true;

        default:
            return false;
    }
}

bool Expression::is_identifier() const {
    switch (expression_type) {
        case ExpressionType::identifier:
        case ExpressionType::type_identifier:
        case ExpressionType::operator_identifier:
        case ExpressionType::type_operator_identifier:
            return true;

        default:
            return false;
    }
}

bool Expression::is_ok_in_layer2() const {
    return (!is_identifier() && !is_illegal());
}

bool Expression::is_ok_in_codegen() const {
    switch (expression_type) {
        case ExpressionType::reference:
        case ExpressionType::call:
        case ExpressionType::string_literal:
        case ExpressionType::numeric_literal:
            return true;

        default:
            return false;
    }
}

bool Expression::is_castable_expression() const {
    return true;
}

Statement::Statement(StatementType statement_type, SourceLocation location)
:statement_type(statement_type), location(location) {
    switch (statement_type) {
        case StatementType::broken:
        case StatementType::illegal:
            value = static_cast<std::string>("");
            break;
        case StatementType::empty:
            value = std::monostate{};
            break;               

        case StatementType::expression_statement:
            break;
        case StatementType::block:
            value = Block{};
            break;  
        case StatementType::let:
            value = Let{};
            break;
        case StatementType::operator_definition:
            value = OperatorStatementValue{};
            break;
        case StatementType::assignment:
            break;          
        case StatementType::return_:
            break;             
        //case StatementType::if:break;
        //case StatementType::else:break;
        //case StatementType::for:break;
        //case StatementType::for_id:break;
        //case StatementType::do_while:break;
        //case StatementType::do_for:break;
        //case StatementType::while/until: break;
        //case StatementType::switch:break;
    }
}


// ----- CALLABLE -----

Callable::Callable(CallableBody body, const std::string& name, 
    std::optional<SourceLocation> location)
:body(body), name(name), location(location) {}

Callable::Callable(CallableBody body, std::optional<SourceLocation> location)
:body(body), name("anonymous callable"), location(location) {}

const Type* Callable::get_type() const {
    switch (body.index()) {
        case 0: // uninitialized
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

bool Callable::is_operator() const {
    return static_cast<bool>(operator_props);
}

bool Callable::is_binary_operator() const {
    if (!is_operator())
        return false;

    return (*operator_props)->is_binary();
}

bool Callable::is_unary_operator() const {
    if (!is_operator())
        return false;

    return (*operator_props)->is_unary();
}

} // namespace AST