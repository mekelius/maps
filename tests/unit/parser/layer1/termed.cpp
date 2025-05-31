#include "doctest.h"

#include <sstream>

#include "mapsc/ast/scope.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/parser/layer1.hh"

using namespace Maps;


TEST_CASE("Should parse parentheses and colons correctly") {
    auto [state, _, types] = CompilationState::create_test_state();
    Scope scope{};
    REQUIRE(scope.empty());

    SUBCASE("123 + (2 + 4 * 7)") {
        std::stringstream source{"123 + (2 + 4 * 7)"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        
        auto termed = result.unparsed_termed_expressions;
        CHECK(termed.size() == 2);

        CHECK(result.top_level_definition);
        CHECK(std::holds_alternative<Expression*>((*result.top_level_definition)->body()));

        auto outer = std::get<Expression*>((*result.top_level_definition)->body());
        CHECK(outer->expression_type == ExpressionType::layer2_expression);
        CHECK(outer->terms().size() == 3);

        Expression* inner = outer->terms().at(2);
        CHECK(inner->expression_type == ExpressionType::layer2_expression);
        CHECK(inner->terms().size() == 5);
    }

    SUBCASE("123 + (((2)) + (4) * 7)") {
        std::stringstream source{"123 + (((2)) + (4) * 7)"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        
        auto termed = result.unparsed_termed_expressions;
        CHECK(termed.size() == 2);

        CHECK(result.top_level_definition);
        CHECK(std::holds_alternative<Expression*>((*result.top_level_definition)->body()));

        auto outer = std::get<Expression*>((*result.top_level_definition)->body());
        CHECK(outer->expression_type == ExpressionType::layer2_expression);
        CHECK(outer->terms().size() == 3);

        Expression* inner = outer->terms().at(2);
        CHECK(inner->expression_type == ExpressionType::layer2_expression);
        CHECK(inner->terms().size() == 5);
    }

    SUBCASE("(123 + (2 + (4) * 7))") {
        std::stringstream source{"(123 + (2 + (4) * 7))"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        
        auto termed = result.unparsed_termed_expressions;
        CHECK(termed.size() == 2);

        CHECK(result.top_level_definition);
        CHECK(std::holds_alternative<Expression*>((*result.top_level_definition)->body()));

        auto outer = std::get<Expression*>((*result.top_level_definition)->body());
        CHECK(outer->expression_type == ExpressionType::layer2_expression);
        CHECK(outer->terms().size() == 3);

        Expression* inner = outer->terms().at(2);
        CHECK(inner->expression_type == ExpressionType::layer2_expression);
        CHECK(inner->terms().size() == 5);
    }

    SUBCASE("123 + 2+4*7") {
        std::stringstream source{"123 + 2+4*7"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);

        auto termed = result.unparsed_termed_expressions;
        CHECK(termed.size() == 2);

        CHECK(result.top_level_definition);
        CHECK(std::holds_alternative<Expression*>((*result.top_level_definition)->body()));

        auto outer = std::get<Expression*>((*result.top_level_definition)->body());
        CHECK(outer->expression_type == ExpressionType::layer2_expression);
        CHECK(outer->terms().size() == 3);

        Expression* inner = outer->terms().at(2);
        CHECK(inner->expression_type == ExpressionType::layer2_expression);
        CHECK(inner->terms().size() == 5);
    }

    SUBCASE("Int 23") {
        std::stringstream source{"Int 23"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);
        CHECK(std::get<Expression*>((*result.top_level_definition)->body())->expression_type == ExpressionType::layer2_expression);

        auto termed = result.unparsed_termed_expressions;

        CHECK(termed.size() == 1);
        CHECK(termed.at(0)->terms().size() == 2);
    }

    SUBCASE("Int : 23") {
        std::stringstream source{"Int : 23"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);
        CHECK(std::get<Expression*>((*result.top_level_definition)->body())->expression_type == ExpressionType::layer2_expression);

        auto termed = result.unparsed_termed_expressions;

        CHECK(termed.size() == 1);
        CHECK(termed.at(0)->terms().size() == 2);
    }

    SUBCASE("Int: 23") {
        std::stringstream source{"Int: 23"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);
        CHECK(std::get<Expression*>((*result.top_level_definition)->body())->expression_type == ExpressionType::layer2_expression);

        auto termed = result.unparsed_termed_expressions;

        CHECK(termed.size() == 1);
        CHECK(termed.at(0)->terms().size() == 2);
    }

    SUBCASE("- 123 + : 2 + 4 * 7") {
        std::stringstream source{"- 123 + : 2 + 4 * 7"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);

        auto termed = result.unparsed_termed_expressions;
        CHECK(termed.size() == 3);

        CHECK(result.top_level_definition);
        CHECK(std::holds_alternative<Expression*>((*result.top_level_definition)->body()));

        auto outer = std::get<Expression*>((*result.top_level_definition)->body());
        CHECK(outer->expression_type == ExpressionType::layer2_expression);
        CHECK(outer->terms().size() == 2);

        Expression* lhs = outer->terms().at(0);
        Expression* rhs = outer->terms().at(1);

        CHECK(lhs->terms().size() == 3);
        CHECK(rhs->terms().size() == 5);
    }

    SUBCASE("Initial identifier") {
        std::stringstream source{"a: b"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);

        auto termed = result.unparsed_termed_expressions;
        CHECK(termed.size() == 1);

        CHECK(result.top_level_definition);
        CHECK(std::holds_alternative<Expression*>((*result.top_level_definition)->body()));

        auto outer = std::get<Expression*>((*result.top_level_definition)->body());
        CHECK(outer->expression_type == ExpressionType::layer2_expression);
        CHECK(outer->terms().size() == 2);
    }

    SUBCASE("Very complex case") {
        std::stringstream source{"a: : 1 2 : - 123 + : 2 + 4 7:2 3 (4) 1 2"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);

        auto termed = result.unparsed_termed_expressions;
        CHECK(termed.size() == 8);

        CHECK(result.top_level_definition);
        CHECK(std::holds_alternative<Expression*>((*result.top_level_definition)->body()));

        auto outer = std::get<Expression*>((*result.top_level_definition)->body());
        CHECK(outer->expression_type == ExpressionType::layer2_expression);
        CHECK(outer->terms().size() == 2);

        Expression* lhs1 = outer->terms().at(0);
        Expression* rhs1 = outer->terms().at(1);
        
        CHECK(lhs1->expression_type == ExpressionType::identifier);
        CHECK(rhs1->terms().size() == 2);


        Expression* lhs2 = rhs1->terms().at(0);
        Expression* rhs2 = rhs1->terms().at(1);

        CHECK(lhs2->terms().size() == 2);
        CHECK(rhs2->terms().size() == 2);


        Expression* lhs3 = rhs2->terms().at(0);
        Expression* rhs3 = rhs2->terms().at(1);

        CHECK(lhs3->terms().size() == 3);
        CHECK(rhs3->terms().size() == 2);


        Expression* lhs4 = rhs3->terms().at(0);
        Expression* rhs4 = rhs3->terms().at(1);

        CHECK(lhs4->terms().size() == 4);
        CHECK(rhs4->terms().size() == 5);
    }
}