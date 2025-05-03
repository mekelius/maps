#include "scope.hh"

#include "mapsc/ast/ast_store.hh"

namespace Maps {

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
    identifiers_in_order_.push_back({name, callable});
    
    return callable;
}

std::optional<Callable*> Scope::create_callable(const std::string& name, SourceLocation location) {
    return create_callable(name, std::monostate{}, location);
}

// quite messy
std::optional<Callable*> Scope::create_binary_operator(const std::string& name, CallableBody body, 
    Precedence precedence, Associativity associativity, SourceLocation location) {

    if (identifier_exists(name))
        return std::nullopt;

    auto operator_props = ast_->create_operator({UnaryFixity::none, BinaryFixity::infix, 
        precedence, associativity, location});
    const Type* type = ast_->types_->get_function_type(Hole, {&Hole, &Hole});
    
    Callable* callable = *create_callable(name, body, location);
    callable->operator_props = operator_props;

    // !!!
    callable->set_type(*type);

    return callable;
}

std::optional<Callable*> Scope::create_unary_operator(const std::string& name, CallableBody body, UnaryFixity fixity, 
    SourceLocation location) {

    if (identifier_exists(name))
        return std::nullopt;

    // TODO: use named identifiers
    auto operator_props = ast_->create_operator({fixity, BinaryFixity::none, 0, Associativity::left,location});
    const Type* type = ast_->types_->get_function_type(Hole, {&Hole});
    
    Callable* callable = *create_callable(name, body, location);
    callable->operator_props = operator_props;
    
    // !!!
    callable->set_type(*type);

    return callable;
}

std::optional<Expression*> Scope::create_reference_expression(const std::string& name, SourceLocation location) {
    std::optional<Callable*> callable = get_identifier(name);

    if (! callable)
        return std::nullopt;

    return create_reference_expression(*callable, location);
}

Expression* Scope::create_reference_expression(Callable* callable, SourceLocation location) {
    return ast_->create_expression(ExpressionType::reference, callable, *callable->get_type(), location);
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
    return ast_->create_expression(ExpressionType::call, CallExpressionValue{callee, args}, *callee->get_type(), 
        location);
}

} // namespace AST