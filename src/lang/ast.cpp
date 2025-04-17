#include "ast.hh"

#include <cassert>

namespace AST {

// ----- EXPRESSION -----

// TODO: clean this up
const std::string& Expression::string_value() const {
    if (std::holds_alternative<Callable*>(value)) {
        // !!! this will cause crashes when lambdas come in
        return std::get<Callable*>(value)->name;
    }
    return std::get<std::string>(value);
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

Type Callable::get_type() const {
    switch (body.index()) {
        case 0: // uninitialized
            return type_ ? *type_ : Hole;
        
        case 1: // expression
            return std::get<Expression*>(body)->type;

        case 2: { // statement
            Statement* statement = std::get<Statement*>(body);

            if (statement->statement_type == StatementType::expression_statement) {
                return std::get<Expression*>(statement->value)->type;
            } 

            return type_ ? *type_ : Hole;
        }
        case 3: // Builtin
            return std::get<Builtin*>(body)->type;

        default:
            assert(false && "unhandled CallableBody in CallableBody::get_type");
            return Hole;
    }
}


// ----- SCOPE -----

std::optional<Callable*> Scope::create_identifier(const std::string& name, CallableBody body,
    std::optional<SourceLocation> location) {

    if (identifier_exists(name)) {
        return std::nullopt;
    }

    Callable* callable = ast_->create_callable(body, name, location);
    identifiers_.insert({name, callable});
    identifiers_in_order.push_back(name);
    
    return callable;
}

std::optional<Callable*> Scope::create_identifier(const std::string& name, SourceLocation location) {
    return create_identifier(name, std::monostate{}, location);
}


bool Scope::identifier_exists(const std::string& name) const {
    return identifiers_.find(name) != identifiers_.end();
}

std::optional<Callable*> Scope::get_identifier(const std::string& name) const {
    auto it = identifiers_.find(name);
    if (it == identifiers_.end())
        return {};

    return it->second;
}

AST::AST() {}

Expression* AST::create_expression(
    ExpressionType expression_type, SourceLocation location, const Type& type) {
    
    expressions_.push_back(std::make_unique<Expression>(expression_type, location, type));
    Expression* expression = expressions_.back().get();

    switch (expression_type) {
        case ExpressionType::string_literal:
            if (expression->type == Hole)
                expression->type = String;
            expression->value = "";
            break;

        case ExpressionType::numeric_literal:
            if (expression->type == Hole)
                expression->type = NumberLiteral;
            expression->value = "";
            break;

        case ExpressionType::call:
            expression->value = CallExpressionValue{};
            break;

        case ExpressionType::deferred_call:
            expression->value = CallExpressionValueDeferred{};
            break;

        case ExpressionType::termed_expression:
            expression->value = TermedExpressionValue{};
            break;

        case ExpressionType::unresolved_identifier:
        case ExpressionType::unresolved_operator:
        case ExpressionType::empty:
        case ExpressionType::syntax_error:
        case ExpressionType::not_implemented:
        case ExpressionType::identifier:
            expression->value = "";
            break;
        
        case ExpressionType::operator_ref:
            expression->value = nullptr;
            break;

        case ExpressionType::tie:
        case ExpressionType::deleted:
            expression->value = std::monostate{};
            break;
            
        case ExpressionType::binary_operator_apply:
            expression->value = BinaryOperatorApplyValue{};
            break;
        
        case ExpressionType::unary_operator_apply:
            expression->value = UnaryOperatorApplyValue{};
            break;
            
        default:
            assert(false && "unhandled expression type in AST::create_expression");
            break;
    }
    return expression;
}

Statement* AST::create_statement(StatementType statement_type, SourceLocation location) {
    statements_.push_back(std::make_unique<Statement>(statement_type, location));
    return statements_.back().get();
}

Callable* AST::create_builtin(BuiltinType builtin_type, const std::string& name, const Type& type) {
    builtins_.push_back(std::make_unique<Builtin>(builtin_type, name, type));
    Builtin* builtin = builtins_.back().get();

    assert(!builtin_functions_->identifier_exists(name) && !builtin_operators_->identifier_exists(name)
    && "tried to redefine an existing builtin");
    switch (builtin_type) {
        case BuiltinType::builtin_function:
            return *builtin_functions_->create_identifier(name, builtin);

            case BuiltinType::builtin_operator:
            return *builtin_operators_->create_identifier(name, builtin);

        default:
            assert(false && "unhandled builtin type in create_builtin");
            return nullptr;
    }
}

Callable* AST::create_callable(CallableBody body, const std::string& name, 
    std::optional<SourceLocation> location) {
    
    callables_.push_back(std::make_unique<Callable>(body, name, location));
    return callables_.back().get();
}

Callable* AST::create_callable(const std::string& name, SourceLocation location) {
    return create_callable(std::monostate{}, name, location);
}

void AST::append_top_level_statement(Statement* statement) {
    root_.push_back(statement);
}


} // namespace AST