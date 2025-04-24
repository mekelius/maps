#include "doctest.h"

#include "../src/lang/type.hh"
#include "../src/lang/ast.hh"

using Maps::Expression, Maps::ExpressionType, Maps::Callable, Maps::BuiltinType;

TEST_CASE("Callables should pass their type correctly") {
    const unsigned int PRECEDENCE = 756;
    Maps::AST ast{};
    
    Maps::Callable* op = ast.create_builtin(BuiltinType::builtin_operator, "*", 
        *ast.types_->get_binary_operator_type(Maps::Void, Maps::Number, Maps::Number, PRECEDENCE));

    CHECK(op->get_type()->precedence() == PRECEDENCE);
}

TEST_CASE("Operator_ref:s should pass their type correctly") {
    const unsigned int PRECEDENCE = 243;
    Maps::AST ast{};
    
    Maps::Callable* op = ast.create_builtin(BuiltinType::builtin_operator, "*", 
        *ast.types_->get_binary_operator_type(Maps::Void, Maps::Number, Maps::Number, PRECEDENCE));

    Maps::Expression* op_ref = ast.create_operator_ref(op, {0,0});
    
    CHECK(op_ref->type->precedence() == PRECEDENCE);
}