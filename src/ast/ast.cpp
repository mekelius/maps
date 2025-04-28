#include "ast.hh"

#include <cassert>

namespace Maps {

AST::AST() {
    root_ = create_callable("root", {0,0});
}

void AST::set_root(CallableBody root) {
    root_->body = root;
}

// ---------- CREATING (AND DELETING) EXPRESSIONS ----------

Expression* AST::create_string_literal(const std::string& value, SourceLocation location) {
    return create_expression(ExpressionType::string_literal, value, String, location);
}

Expression* AST::create_numeric_literal(const std::string& value, SourceLocation location) {
    return create_expression(ExpressionType::numeric_literal, value, NumberLiteral, location);
}

Expression* AST::create_identifier_expression(const std::string& value, SourceLocation location) {
    Expression* expression = create_expression(ExpressionType::identifier, value, Hole, location);
    unresolved_identifiers_.push_back(expression);
    return expression;
}
Expression* AST::create_type_identifier_expression(const std::string& value, SourceLocation location) {
    Expression* expression = create_expression(ExpressionType::type_identifier, value, Hole, location);
    unresolved_identifiers_.push_back(expression);
    return expression;
}
Expression* AST::create_operator_expression(const std::string& value, SourceLocation location) {
    Expression* expression = create_expression(ExpressionType::operator_identifier, value, Hole, location);
    unresolved_identifiers_.push_back(expression);
    return expression;
}

Expression* AST::create_type_operator_expression(const std::string& value, SourceLocation location) {
    Expression* expression = create_expression(ExpressionType::type_operator_identifier, value, Void, location);
    unresolved_type_identifiers_.push_back(expression);
    return expression;
}

Expression* AST::create_termed_expression(std::vector<Expression*>&& terms, SourceLocation location) {
    return create_expression(ExpressionType::termed_expression, terms, Hole, location);
}

Expression* AST::create_type_reference(const Type* type, SourceLocation location) {
    return create_expression(ExpressionType::type_reference, type, Void, location);
}

std::optional<Expression*> AST::create_operator_ref(const std::string& name, SourceLocation location) {
    // TODO: check user_defined operators as well
    std::optional<Callable*> callable = builtins_scope_->get_identifier(name);
    
    if (!callable)
        return std::nullopt;
    
    return create_operator_ref(*callable, location);
}

Expression* AST::create_operator_ref(Callable* callable, SourceLocation location) {
    assert(callable->is_operator() && "AST::create_operator_ref called with not an operator");

    return create_expression(ExpressionType::operator_reference, callable, *callable->get_type(), location);
}

// valueless expression types are tie, empty, syntax_error and not_implemented
Expression* AST::create_valueless_expression(ExpressionType expression_type, SourceLocation location) {
    return create_expression(expression_type, std::monostate{}, Absurd, location);
}
Expression* AST::create_missing_argument(const Type& type, SourceLocation location) {
    return create_expression(ExpressionType::missing_arg, std::monostate{}, type, location);
}

void AST::delete_expression(Expression* expression) {
    expression->expression_type = ExpressionType::deleted;
}

// ---------- CREATING OTHER THINGS ----------

Statement* AST::create_statement(StatementType statement_type, SourceLocation location) {
    statements_.push_back(std::make_unique<Statement>(statement_type, location));
    return statements_.back().get();
}

Callable* AST::create_builtin(const std::string& name, const Type& type) {
    builtins_.push_back(std::make_unique<Builtin>(name, &type));
    Builtin* builtin = builtins_.back().get();

    assert(!builtins_scope_->identifier_exists(name)
        && "tried to redefine an existing builtin");
    
    return *builtins_scope_->create_callable(name, builtin);
}

Callable* AST::create_builtin_binary_operator(const std::string& name, const Type& type, 
    Precedence precedence, Associativity Associativity) {
    
    assert(type.arity() >= 2 && "AST::create_builtin_binary_operator called with arity < 2");

    Callable* callable = create_builtin(name, type);
    callable->operator_props = create_operator({
        UnaryFixity::none, BinaryFixity::infix, precedence, Associativity});

    return callable;
}

Callable* AST::create_builtin_unary_operator(const std::string& name, const Type& type, UnaryFixity fixity) {
    
    assert(type.arity() >= 1 && "AST::create_builtin_unary_operator called with arity < 1");

    Callable* callable = create_builtin(name, type);
    callable->operator_props = create_operator({fixity, BinaryFixity::none});

    return callable;
}

// --------- PRIVATE NODE MANAGEMENT ---------

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

Operator* AST::create_operator(const Operator&& operator_props) {
    operators_.push_back(std::make_unique<Operator>(operator_props));
    return operators_.back().get();
}

} // namespace AST