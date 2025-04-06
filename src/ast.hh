#ifndef __AST_HH
#define __AST_HH

#include <vector>
#include <unordered_map>
#include <tuple>
#include <string>
#include <memory>
#include <string_view>
#include <optional>
#include <cassert>

namespace AST {

// class for booleans we may or may not know yet
enum class DeferredBool {
    true_,
    false_,
    unknown,
};

struct Type {
    bool is_native = false;
    std::string_view name;
    DeferredBool is_numeric = DeferredBool::false_;
    DeferredBool is_integral = DeferredBool::false_;
};

constexpr Type Int {
    true,
    "Int",
    DeferredBool::true_,
    DeferredBool::true_,
};

constexpr Type Boolean {
    true,
    "Boolean",
    DeferredBool::false_,
    DeferredBool::false_,
};

constexpr Type String {
    true,
    "String",
    DeferredBool::false_,
    DeferredBool::false_,
};

constexpr Type Void {
    true,
    "Void",
    DeferredBool::false_,
    DeferredBool::false_,
};

constexpr Type Hole {
    false,
    "Hole",
    DeferredBool::unknown,
    DeferredBool::unknown,
};

// a number who's type hasn't yet been determined
constexpr Type NumberLiteral {
    false,
    "NumberLiteral",
    DeferredBool::true_,
    DeferredBool::unknown,
};

enum class StatementType {
    // let,                    // 
    assign_,
    return_,
    expression
};

enum class ExpressionType {
    string_literal,         // literal: Value value
    numeric_literal,
    call,                   // call: Callee identifier, [Expression] args
    // binary,              // binary: Operator operator, Expression lhs, Expression rhs
    // block,                  // block: [Statement] statements
    callable_expression,
    native_function,
    native_operator,
    function_body,
    not_implemented,
};

struct Expression {
    Expression(ExpressionType expr_type, const Type* type): expression_type(expr_type), type(type) {};
    
    ExpressionType expression_type;
    const Type* type = &Hole;
    std::tuple<std::string, std::vector<Expression*>> call_expr = {"", {}};
    std::string string_value = "";
};

struct Callable {
    Callable(const std::string& name, Expression* expression, std::vector<const Type*>& arg_types)
    : name(name), expression(expression), arg_types(arg_types) {
        assert(expression && "Tried to initialize callable with a nullptr expression");
    }

    std::string name;
    Expression* expression;
    std::vector<const Type*> arg_types;
};

struct Scope {
    std::unordered_map<std::string, Callable*> identifiers;
};

class AST {
  public:
    bool valid = true;

    Expression* create_expression(ExpressionType expression_type, const Type* type = &Void);

    // returns nullopt if the name is taken
    std::optional<Callable*> create_callable(
        const std::string& name, 
        Expression* expression,
        std::vector<const Type*> arg_types = {}
    );

    bool name_free(const std::string& name) const;
    std::optional<Callable*> get_identifier(const std::string& name);
    // std::unordered_map<> call_sites_
    
    Scope global = {};

  private:
    // caller is responsible for checking if the name is free with name_free
    void create_identifier(const std::string& name, Callable* callable);

    std::vector<std::unique_ptr<Callable>> callables_ = {};
    std::vector<std::unique_ptr<Expression>> expressions_ = {};
    std::unordered_map<std::string, Callable*> identifiers_ = {};
};

bool init_builtin_callables(AST& ast);

} // namespace AST
    
      

#endif