#include "ast.hh"

#include <cassert>

namespace AST {

Expression* AST::create_expression(ExpressionType expression_type, const Type* type) {
    expressions_.push_back(std::make_unique<Expression>(expression_type, type));
    Expression* expression = expressions_.back().get();

    switch (expression_type) {
        case ExpressionType::string_literal:
            if (expression->type == &Hole)
                expression->type = &String;
            expression->value = "";
            break;

        case ExpressionType::numeric_literal:
            if (expression->type == &Hole)
                expression->type = &NumberLiteral;
            expression->value = "";
            break;

        case ExpressionType::call:
            expression->value = CallExpressionValue{};
            break;

        case ExpressionType::deferred_call:
            expression->value = CallExpressionValueDeferred{};
            break;

        case ExpressionType::native_function:
            // TODO: infer type
            expression->value = "";
            break;

        case ExpressionType::native_operator:
            // TODO: create enum of native operators
            expression->value = "";
            break;

        case ExpressionType::termed_expression:
            expression->value = TermedExpressionValue{};
            break;

        case ExpressionType::unresolved_identifier:
        case ExpressionType::unresolved_operator:
        case ExpressionType::syntax_error:
        case ExpressionType::not_implemented:
            expression->value = "";
            break;

        case ExpressionType::tie:
        case ExpressionType::deleted:
            expression->value = std::monostate{};
            break;
            
        default:
            assert(false && "unhandled expression type in AST::create_expression");
            break;
    }
    return expression;
}

Statement* AST::create_statement(Statement&& statement) {
    statements_.push_back(std::make_unique<Statement>(std::move(statement)));
    return statements_.back().get();
}

void AST::append_top_level_statement(Statement* statement) {
    root_.push_back(statement);
}

std::optional<Callable*> AST::create_callable(
    const std::string& name,
    CallableBody body,
    const Type* return_type,
    std::vector<const Type*> arg_types
) {
    if (!name_free(name)) {
        return std::nullopt;
    }

    callables_.push_back(std::make_unique<Callable>(name, body, return_type, arg_types));
    Callable* callable = callables_.back().get();
    create_identifier(name, callable);
    global_.identifiers.insert({name, callable});
    global_.identifiers_in_order.push_back(name);
    
    return callable;
}

void AST::create_identifier(const std::string& name, Callable* callable) {
    identifiers_.insert({name, callable});
}

bool AST::name_free(const std::string& name) const {
    return identifiers_.find(name) == identifiers_.end();
}

std::optional<Callable*> AST::get_identifier(const std::string& name) {
    auto it = identifiers_.find(name);
    if (it == identifiers_.end())
        return {};

    return it->second;
}

bool AST::init_builtin_callables() {
    auto print_expr = create_expression(ExpressionType::native_function, &Void);
    auto print = create_callable("print", print_expr, &Void, { &String });
    
    if (!print)
        return false;

    (*print)->name = "print";

    return true;
}

} // namespace AST