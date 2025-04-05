#ifndef __AST_HH
#define __AST_HH

#include <vector>
#include <unordered_map>
#include <tuple>
#include <string>
#include <memory>

namespace AST {

struct Type {
    bool is_native = false;
    std::string name;
    bool is_numeric = false;
    bool is_integral = false;
};

// TODO: should be constexpr
const Type Int {
    false,
    "Int",
    true,
    true
};

// struct Mapping: public Type {
    // Type* domain;
    // Type* codomain;
// };

// struct ConcreteType: public Type {};
// struct ParameterizedType: public Type {
//     std::vector<TypeExpression> 
// };

// struct Function: public Mapping {};

enum class IdentifierClass {
    variable,
    builtin
    // type,
};

// enum class Operator


enum class StatementType {
    // let,                    // 
    assign_,
    return_,
    expression
};

struct Identifier {
    // IdentifierClass identifier_class;
    // std::unique_ptr<Expression> expression;
};

struct Operator {};

enum class ExpressionType {
    string_literal,         // literal: Value value
    call,                   // call: Callee identifier, [Expression] args
    // binary,              // binary: Operator operator, Expression lhs, Expression rhs
    // block,                  // block: [Statement] statements
};

struct Expression {
    Expression(ExpressionType expr_type): type(expr_type) {};
    // ~Expression() {
    //     switch (type) {
    //         case ExpressionType::string_literal:
    //             string_value.~basic_string();
    //             break;
            
    //         case ExpressionType::call:
    //             std::get<1>(call_expr).~vector();
    //             break;
    //     }
    // };

    ExpressionType type;

    // union {
        std::tuple<std::string, std::vector<Expression*>> call_expr = {"", {}};
        std::string string_value = "";
        // std::tuple<Expression*, Operator*, Expression*> binary_expr;
    // };

};

struct Callable {
    std::string name;
    unsigned int arity = 0;
    Expression* expression;
};


class AST {
  public:
    bool valid = true;

    Expression* root;
    Expression* entry_point;
    std::unordered_map<std::string, Expression*> identifiers;
    
    Expression* create_expression(ExpressionType type);
    std::vector<Callable*> callables_;
    // Callable callables;
    // std::unordered_map<> call_sites_
    
  private:
    std::vector<std::unique_ptr<Expression>> expressions_ = {};
};

} // namespace AST
    
      

#endif