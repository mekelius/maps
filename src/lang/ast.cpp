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

} // namespace AST