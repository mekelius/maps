#include "ast_store.hh"

#include "mapsc/builtins.hh"
#include "mapsc/ast/builtin.hh"

#include <cassert>

namespace Maps {

AST_Store::AST_Store() {
    root_ = create_callable("root", {0,0});
}

bool AST_Store::init_builtins() {
    // !!! this isn't good
    return Maps::init_builtins(*this);
}

void AST_Store::set_root(CallableBody root) {
    // TODO: delete the old
    // delete_node(root_);
    root_->body = root;
}

bool AST_Store::empty() const {
    return (
        statements_.empty() &&
        expressions_.empty() &&
        builtins_.empty() &&
        operators_.empty() &&

        // !!! this isn't good
        callables_.size() == 1 &&
        *callables_.back() == *root_
    );
}

size_t AST_Store::size() const {
    return (
        statements_.size() +
        expressions_.size() +
        builtins_.size() +
        operators_.size() +

        // !!! this isn't good
        callables_.size() - 1
    );
}

// ---------- CREATING (AND DELETING) EXPRESSIONS ----------

Expression* AST_Store::create_string_literal(const std::string& value, SourceLocation location) {
    return create_expression(ExpressionType::string_literal, value, String, location);
}

Expression* AST_Store::create_numeric_literal(const std::string& value, SourceLocation location) {
    return create_expression(ExpressionType::numeric_literal, value, NumberLiteral, location);
}

Expression* AST_Store::create_identifier_expression(const std::string& value, SourceLocation location) {
    Expression* expression = create_expression(ExpressionType::identifier, value, Hole, location);
    unresolved_identifiers_.push_back(expression);
    return expression;
}
Expression* AST_Store::create_type_identifier_expression(const std::string& value, SourceLocation location) {
    Expression* expression = create_expression(ExpressionType::type_identifier, value, Hole, location);
    unresolved_identifiers_.push_back(expression);
    return expression;
}
Expression* AST_Store::create_operator_expression(const std::string& value, SourceLocation location) {
    Expression* expression = create_expression(ExpressionType::operator_identifier, value, Hole, location);
    unresolved_identifiers_.push_back(expression);
    return expression;
}

Expression* AST_Store::create_type_operator_expression(const std::string& value, SourceLocation location) {
    Expression* expression = create_expression(ExpressionType::type_operator_identifier, value, Void, location);
    unresolved_type_identifiers_.push_back(expression);
    return expression;
}

Expression* AST_Store::create_termed_expression(std::vector<Expression*>&& terms, SourceLocation location) {
    return create_expression(ExpressionType::termed_expression, TermedExpressionValue{terms}, Hole, location);
}

Expression* AST_Store::create_type_reference(const Type* type, SourceLocation location) {
    return create_expression(ExpressionType::type_reference, type, Void, location);
}

std::optional<Expression*> AST_Store::create_operator_ref(const std::string& name, SourceLocation location) {
    // TODO: check user_defined operators as well
    std::optional<Callable*> callable = builtins_scope_->get_identifier(name);
    
    if (!callable)
        return std::nullopt;
    
    return create_operator_ref(*callable, location);
}

Expression* AST_Store::create_operator_ref(Callable* callable, SourceLocation location) {
    assert(callable->is_operator() && "AST::create_operator_ref called with not an operator");

    return create_expression(ExpressionType::operator_reference, callable, *callable->get_type(), location);
}

// valueless expression types are tie, empty, syntax_error and not_implemented
Expression* AST_Store::create_valueless_expression(ExpressionType expression_type, SourceLocation location) {
    return create_expression(expression_type, std::monostate{}, Absurd, location);
}
Expression* AST_Store::create_missing_argument(const Type& type, SourceLocation location) {
    return create_expression(ExpressionType::missing_arg, std::monostate{}, type, location);
}

void AST_Store::delete_expression(Expression* expression) {
    expression->expression_type = ExpressionType::deleted;
}
void AST_Store::delete_expression_recursive(Expression* expression) {
    assert(false && "not implemented");
}

void AST_Store::delete_statement(Statement* statement) {
    statement->statement_type = StatementType::deleted;
}
void AST_Store::delete_statement_recursive(Statement* statement) {
    assert(false && "not implemented");
}

// ---------- CREATING OTHER THINGS ----------

Statement* AST_Store::create_statement(StatementType statement_type, SourceLocation location) {
    statements_.push_back(std::make_unique<Statement>(statement_type, location));
    return statements_.back().get();
}

Callable* AST_Store::create_builtin(const std::string& name, const Type& type) {
    builtins_.push_back(std::make_unique<Builtin>(name, &type));
    Builtin* builtin = builtins_.back().get();

    assert(!builtins_scope_->identifier_exists(name)
        && "tried to redefine an existing builtin");
    
    return *builtins_scope_->create_callable(name, builtin);
}

Callable* AST_Store::create_builtin_binary_operator(const std::string& name, const Type& type, 
    Precedence precedence, Associativity Associativity) {
    
    assert(type.arity() >= 2 && "AST::create_builtin_binary_operator called with arity < 2");

    Callable* callable = create_builtin(name, type);
    callable->operator_props = create_operator({
        UnaryFixity::none, BinaryFixity::infix, precedence, Associativity});

    return callable;
}

Callable* AST_Store::create_builtin_unary_operator(const std::string& name, const Type& type, UnaryFixity fixity) {
    
    assert(type.arity() >= 1 && "AST::create_builtin_unary_operator called with arity < 1");

    Callable* callable = create_builtin(name, type);
    callable->operator_props = create_operator({fixity, BinaryFixity::none});

    return callable;
}

// --------- PRIVATE NODE MANAGEMENT ---------

Expression* AST_Store::create_expression(ExpressionType expression_type, 
    ExpressionValue value, const Type& type, SourceLocation location) {        

    expressions_.push_back(std::make_unique<Expression>(expression_type, location, value, &type));
    Expression* expression = expressions_.back().get();

    expression->value = value;

    return expression;
}

Callable* AST_Store::create_callable(CallableBody body, const std::string& name, 
    std::optional<SourceLocation> location) {
    
    callables_.push_back(std::make_unique<Callable>(body, name, location));
    return callables_.back().get();
}

Callable* AST_Store::create_callable(const std::string& name, SourceLocation location) {
    return create_callable(std::monostate{}, name, location);
}

Operator* AST_Store::create_operator(const Operator&& operator_props) {
    operators_.push_back(std::make_unique<Operator>(operator_props));
    return operators_.back().get();
}

} // namespace AST