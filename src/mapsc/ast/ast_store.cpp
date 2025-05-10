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

Expression* AST_Store::allocate_expression(const Expression&& expression) {        
    expressions_.push_back(std::make_unique<Expression>(expression));
    return expressions_.back().get();
}

Statement* AST_Store::allocate_statement(const Statement&& statement) {
    statements_.push_back(std::make_unique<Statement>(statement));
    return statements_.back().get();
}

// ---------- CREATING OTHER THINGS ----------

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