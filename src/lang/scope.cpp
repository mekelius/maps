#include "scope.hh"

#include "ast.hh"

namespace AST {

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

std::optional<Callable*> Scope::create_binary_operator(const std::string& name, CallableBody body, 
    unsigned int precedence, Associativity associativity, SourceLocation location) {

    if (identifier_exists(name))
        return std::nullopt;

    Type type = create_binary_operator_type(Hole, Hole, Hole, precedence, associativity);

    Callable* callable = *create_callable(name, body, location);
    // !!! this is pretty hacky
    callable->set_type(type);

    return callable;
}

std::optional<Callable*> Scope::create_unary_operator(const std::string& name, CallableBody body, Fixity fixity, 
    SourceLocation location) {

    if (identifier_exists(name))
        return std::nullopt;

    Type type = create_unary_operator_type(Hole, Hole, fixity);
    
    Callable* callable = *create_callable(name, body, location);
    // !!! this is pretty hacky
    callable->set_type(type);

    return callable;
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

} // namespace AST