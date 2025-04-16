#include "doctest.h"

#include "../src/lang/ast.hh"
#include "../src/parsing/parser_layer2.hh"

#include "test_helpers.hh"

using AST::Expression, AST::ExpressionType, AST::Callable;

TEST_CASE("TermedExpressionParser should replace a single value term with that value") {
    AST::AST ast{};

    Expression* expr = ast.create_expression(ExpressionType::termed_expression, {0,0});

    REQUIRE(expr->terms().size() == 0);

    SUBCASE("String value") {
        Expression* value = ast.create_expression(ExpressionType::string_literal, {0,0});
        value->value = "TEST_STRING:oasrpkorsapok";
        expr->terms().push_back(value);
        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::string_literal);
        CHECK(expr->string_value() == value->string_value());
    }

    SUBCASE("Number value") {
        Expression* value = ast.create_expression(ExpressionType::numeric_literal, {0,0});
        value->value = "234.52";
        expr->terms().push_back(value);
        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::numeric_literal);
        CHECK(expr->string_value() == value->string_value());
    }
}

TEST_CASE("TermedExpressionParser should replace an empty termed expression with ExpressionType::empty") {
    AST::AST ast{};
    Expression* expr = ast.create_expression(ExpressionType::termed_expression, {0,0});
    TermedExpressionParser{&ast, expr}.run();

    CHECK(expr->expression_type == ExpressionType::empty);
}

TEST_CASE("TermedExpressionParser should handle binop expressions") {
    AST::AST ast{};
    Expression* expr = ast.create_expression(ExpressionType::termed_expression, {0,0});

    Expression* val1 = ast.create_expression(ExpressionType::numeric_literal, {0,0});
    val1->value = "23";
    Expression* val2 = ast.create_expression(ExpressionType::numeric_literal, {0,0});
    val2->value = "12";

    // create the operator
    auto [op1_ref, op1] = create_operator_helper(ast, "+");

    expr->value = std::vector<Expression*>{val1, op1_ref, val2};

    REQUIRE(expr->terms().size() == 3);

    SUBCASE("Simple expression") {
        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::binary_operator_apply);
        // NOTE: the operator_ref should be unwrapped
        CHECK(std::tie(op1, val1, val2) == expr->binop_apply_value());
    }

    SUBCASE("should handle higher precedence first") {
        // create operator with lower precedence
        auto [op2_ref, op2] = create_operator_helper(ast, "*", 1);
        expr->terms().push_back(op2_ref);

        Expression* val3 = ast.create_expression(ExpressionType::numeric_literal, {0,0});
        val3->value = "235";
        expr->terms().push_back(val3);

        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::binary_operator_apply);
        auto [outer_op, outer_lhs, outer_rhs] = expr->binop_apply_value();

        CHECK(outer_op == op2);
        CHECK(outer_rhs == val3);

        // check the lhs
        CHECK(outer_lhs->expression_type == ExpressionType::binary_operator_apply);
        auto [inner_op, inner_lhs, inner_rhs] = outer_lhs->binop_apply_value();

        CHECK(inner_op == op1);
        CHECK(inner_lhs == val1);
        CHECK(inner_rhs == val2);
    }

    SUBCASE("should handle lower precedence first") {
        // create operator with lower precedence
        auto [op2_ref, op2] = create_operator_helper(ast, "*", 999);
        expr->terms().push_back(op2_ref);

        Expression* val3 = ast.create_expression(ExpressionType::numeric_literal, {0,0});
        val3->value = "235";
        expr->terms().push_back(val3);

        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::binary_operator_apply);
        auto [outer_op, outer_lhs, outer_rhs] = expr->binop_apply_value();

        CHECK(outer_op == op1);
        CHECK(outer_lhs == val1);

        // check the lhs
        CHECK(outer_rhs->expression_type == ExpressionType::binary_operator_apply);
        auto [inner_op, inner_lhs, inner_rhs] = outer_rhs->binop_apply_value();

        CHECK(inner_op == op2);
        CHECK(inner_lhs == val2);
        CHECK(inner_rhs == val3);
    }
}
