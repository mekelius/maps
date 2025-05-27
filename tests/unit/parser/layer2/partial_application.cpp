#include "doctest.h"

#include <sstream>

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/parser/layer2.hh"
#include "mapsc/logging_options.hh"

using namespace Maps;
using namespace std;


namespace {

inline tuple<Expression*, Definition*> create_operator_helper(CompilationState& state, 
    const string& op_string, unsigned int precedence = 500) {

    const Type* type = state.types_->get_function_type(&Number, {&Number, &Number}, true);
    Definition* op_definition = state.ast_store_->allocate_operator(
        RT_Operator::create_binary(op_string, External{}, type, precedence, 
            RT_Operator::Associativity::left, true, TSL));

    Expression* op_ref = Expression::operator_reference(*state.ast_store_, op_definition, {0,0});

    return {op_ref, op_definition};
}

} // namespace


TEST_CASE("Should handle partial application of binary operators") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;

    auto [op_ref, op] = create_operator_helper(state, "-");
    auto val = Expression::numeric_literal(ast, "34", {0,0});

    SUBCASE("left") {
        Expression* expr = Expression::termed_testing(ast, {op_ref, val}, {0,0});
        
        run_layer2(state, expr);
        
        CHECK(expr->expression_type == ExpressionType::partial_binop_call_left);
        auto [callee, args] = expr->call_value();
        CHECK(callee == op);
        CHECK(args.size() == 2);
        CHECK(args.at(0)->expression_type == ExpressionType::missing_arg);
        CHECK(args.at(1) == val);
    }

    SUBCASE("right") {
        Expression* expr = Expression::termed_testing(ast, {val, op_ref}, {0,0});
        
        run_layer2(state, expr);
        
        CHECK(expr->expression_type == ExpressionType::partial_binop_call_right);
        auto [callee, args] = expr->call_value();
        CHECK(callee == op);
        CHECK(args.size() == 2);
        CHECK(args.at(0) == val);
        CHECK(args.at(1)->expression_type == ExpressionType::missing_arg);
    }
}

TEST_CASE("Should set the type on a non-partial call expression to the return type") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;

    const FunctionType* IntString = types->get_function_type(&String, {&Int}, false);

    auto test_f_expr = Expression{ExpressionType::known_value, "qwe", IntString, TSL};
    auto test_f = RT_Definition("test_f", &test_f_expr, true, TSL);

    auto arg = Expression{ExpressionType::numeric_literal, "3", &NumberLiteral, TSL};
    auto reference = Expression::reference(ast, &test_f, TSL);

    auto expr = Expression::termed_testing(ast, {reference, &arg}, TSL);

    run_layer2(state, expr);

    CHECK(expr->expression_type == ExpressionType::call);
    CHECK(*expr->type == String);
    CHECK(expr->call_value() == CallExpressionValue{&test_f, {&arg}});
}


TEST_CASE("Should set the type on a non-partial \"operator expression\" to the return type") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;

    const FunctionType* IntString = types->get_function_type(&String, {&Int, &Int}, false);

    auto test_op_expr = Expression{ExpressionType::known_value, "jii", IntString, TSL};
    auto test_op = RT_Operator::create_binary(">=?", &test_op_expr, 5, 
        Operator::Associativity::left, true, TSL);

    auto lhs = Expression{ExpressionType::numeric_literal, "3", &NumberLiteral, TSL};
    auto rhs = Expression{ExpressionType::numeric_literal, "7", &NumberLiteral, TSL};

    auto reference = Expression::operator_reference(ast, &test_op, TSL);

    auto expr = Expression::termed_testing(ast, {&lhs, reference, &rhs}, TSL);

    run_layer2(state, expr);

    CHECK(expr->expression_type == ExpressionType::call);
    CHECK(expr->call_value() == CallExpressionValue{&test_op, {&lhs, &rhs}});
    CHECK(*expr->type == String);
}
