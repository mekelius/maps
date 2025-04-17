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

bool Scope::identifier_exists(const std::string& name) const {
    return identifiers_.find(name) != identifiers_.end();
}

std::optional<Callable*> Scope::get_identifier(const std::string& name) const {
    auto it = identifiers_.find(name);
    if (it == identifiers_.end())
        return {};

    return it->second;
}

std::optional<Callable*> Scope::create_callable(const std::string& name, CallableBody body,
    std::optional<SourceLocation> location) {

    if (identifier_exists(name)) {
        return std::nullopt;
    }

    Callable* callable = ast_->create_callable(body, name, location);
    identifiers_.insert({name, callable});
    identifiers_in_order_.push_back(name);
    
    return callable;
}

std::optional<Callable*> Scope::create_callable(const std::string& name, SourceLocation location) {
    return create_callable(name, std::monostate{}, location);
}

std::optional<Expression*> Scope::create_reference_expression(const std::string& name, SourceLocation location) {
    std::optional<Callable*> callable = get_identifier(name);

    if (! callable)
        return std::nullopt;

    return create_reference_expression(*callable, location);
}

Expression* Scope::create_reference_expression(Callable* callable, SourceLocation location) {
    return ast_->create_expression(ExpressionType::reference, callable, callable->get_type(), location);
}

std::optional<Expression*> Scope::create_call_expression(
    const std::string& callee_name, std::vector<Expression*> args, SourceLocation location /*, expected return type?*/) {
    
    std::optional<Callable*> callee = get_identifier(callee_name);
    
    if (!callee)
        return std::nullopt;

    return create_call_expression(*callee, args, location);
}

Expression* Scope::create_call_expression(Callable* callee, std::vector<Expression*> args, 
        SourceLocation location /*, expected return type?*/) {
    return ast_->create_expression(ExpressionType::call, CallExpressionValue{callee, args}, callee->get_type(), 
        location);
}

// ----- AST -----

AST::AST() {}

Expression* AST::create_string_literal(const std::string& value, SourceLocation location) {
    return create_expression(ExpressionType::string_literal, value, String, location);
}

Expression* AST::create_numeric_literal(const std::string& value, SourceLocation location) {
    return create_expression(ExpressionType::numeric_literal, value, NumberLiteral, location);
}

Expression* AST::create_identifier_expression(const std::string& value, SourceLocation location) {
    Expression* expression = create_expression(ExpressionType::identifier, value, Hole, location);
    unresolved_identifiers_and_operators.push_back(expression);
    return expression;
}
Expression* AST::create_operator_expression(const std::string& value, SourceLocation location) {
    Expression* expression = create_expression(ExpressionType::operator_e, value, Hole, location);
    unresolved_identifiers_and_operators.push_back(expression);
    return expression;
}

Expression* AST::create_termed_expression(std::vector<Expression*>&& terms, SourceLocation location) {
    return create_expression(ExpressionType::termed_expression, terms, Hole, location);
}

std::optional<Expression*> AST::create_operator_ref(const std::string& name, SourceLocation location) {
    // TODO: check user_defined operators as well
    std::optional<Callable*> callable = builtin_operators_->get_identifier(name);
    
    if (!callable)
        return std::nullopt;
    
    return create_operator_ref(*callable, location);
}
Expression* AST::create_operator_ref(Callable* callable, SourceLocation location) {
    return create_expression(ExpressionType::operator_ref, callable, callable->get_type(), location);
}

// valueless expression types are tie, empty, syntax_error and not_implemented
Expression* AST::create_valueless_expression(ExpressionType expression_type, SourceLocation location) {
    return create_expression(expression_type, std::monostate{}, Absurd, location);
}

void AST::delete_expression(Expression* expression) {
    expression->expression_type = ExpressionType::deleted;
}

Statement* AST::create_statement(StatementType statement_type, SourceLocation location) {
    statements_.push_back(std::make_unique<Statement>(statement_type, location));
    return statements_.back().get();
}

void AST::append_top_level_statement(Statement* statement) {
    root_.push_back(statement);
}

Callable* AST::create_builtin(BuiltinType builtin_type, const std::string& name, const Type& type) {
    builtins_.push_back(std::make_unique<Builtin>(builtin_type, name, type));
    Builtin* builtin = builtins_.back().get();

    assert(!builtin_functions_->identifier_exists(name) && !builtin_operators_->identifier_exists(name)
    && "tried to redefine an existing builtin");
    switch (builtin_type) {
        case BuiltinType::builtin_function:
            return *builtin_functions_->create_callable(name, builtin);

        case BuiltinType::builtin_operator:
            return *builtin_operators_->create_callable(name, builtin);

        default:
            assert(false && "unhandled builtin type in create_builtin");
            return nullptr;
    }
}

Expression* AST::create_expression(ExpressionType expression_type, 
    ExpressionValue value, const Type& type, SourceLocation location) {        

    expressions_.push_back(std::make_unique<Expression>(expression_type, location, type));
    Expression* expression = expressions_.back().get();

    expression->value = value;

    return expression;
}

Callable* AST::create_callable(CallableBody body, const std::string& name, 
    std::optional<SourceLocation> location) {
    
    callables_.push_back(std::make_unique<Callable>(body, name, location));
    return callables_.back().get();
}

Callable* AST::create_callable(const std::string& name, SourceLocation location) {
    return create_callable(std::monostate{}, name, location);
}

} // namespace AST