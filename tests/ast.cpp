#include "doctest.h"

#include "mapsc/types/type.hh"
#include "mapsc/ast/ast.hh"

using Maps::Expression, Maps::ExpressionType, Maps::Callable;

TEST_CASE("AST should be empty when created") {
    Maps::AST_Store ast{};
    CHECK(ast.empty());
    CHECK(ast.size() == 0);
}

TEST_CASE("Callables should pass their type correctly") {
    const unsigned int PRECEDENCE = 756;
    Maps::AST_Store ast{};
    
    Maps::Callable* op = ast.create_builtin_binary_operator("*", 
        *ast.types_->get_function_type(Maps::Void, {&Maps::Number, &Maps::Number}), PRECEDENCE);

    CHECK(op->is_binary_operator());
    CHECK((*op->operator_props)->precedence == PRECEDENCE);
}

TEST_CASE("Operator_ref:s should pass their type correctly") {
    const unsigned int PRECEDENCE = 243;
    Maps::AST_Store ast{};
    
    Maps::Callable* op = ast.create_builtin_binary_operator("*", 
        *ast.types_->get_function_type(Maps::Void, {&Maps::Number, &Maps::Number}), PRECEDENCE);

    Maps::Expression* op_ref = ast.create_operator_ref(op, {0,0});

    CHECK(op_ref->reference_value()->is_binary_operator());
    
    CHECK((*op_ref->reference_value()->operator_props)->precedence == PRECEDENCE);
}