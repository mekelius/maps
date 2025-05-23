#include "doctest.h"

#include <sstream>

#include "mapsc/parser/layer1.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;

TEST_CASE("Should parse lambdas") {
    auto [state, types, _] = CompilationState::create_test_state();
    RT_Scope scope{};

    REQUIRE(state.ast_store_->empty());
    REQUIRE(scope.empty());

    SUBCASE("\\x -> x") {
        std::stringstream source{"\\x -> x"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto expression = std::get<const Expression*>((*result.top_level_definition)->const_body());
        CHECK(expression->expression_type == ExpressionType::lambda);
        CHECK(expression->type->is_function());
        CHECK(expression->type->is_pure());
        CHECK(expression->type->arity() == 1);
    }

    SUBCASE("\\x => x") {
        std::stringstream source{"\\x => x"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto expression = std::get<const Expression*>((*result.top_level_definition)->const_body());
        CHECK(expression->expression_type == ExpressionType::lambda);
        CHECK(expression->type->is_function());
        CHECK(!expression->type->is_pure());
        CHECK(expression->type->arity() == 1);
    }

    SUBCASE("\\x y -> x + y") {
        std::stringstream source{"\\x y -> x + y"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto expression = std::get<const Expression*>((*result.top_level_definition)->const_body());
        CHECK(expression->expression_type == ExpressionType::lambda);
        CHECK(expression->type->is_function());
        CHECK(expression->type->is_pure());
        CHECK(expression->type->arity() == 2);

        auto& body = expression->lambda_value().body;
        CHECK(std::holds_alternative<Expression*>(body));

        auto body_expression = std::get<Expression*>(body);
        CHECK(body_expression->expression_type == ExpressionType::termed_expression);
        CHECK(body_expression->terms().size() == 3);
        CHECK(body_expression->terms().at(0)->expression_type == ExpressionType::identifier);
        CHECK(body_expression->terms().at(1)->expression_type == ExpressionType::operator_identifier);
        CHECK(body_expression->terms().at(2)->expression_type == ExpressionType::identifier);
    }

    SUBCASE("\\x => { print(x); return x + 2 }") {
        std::stringstream source{"\\x => { print(x); return x + 2 }"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto expression = std::get<const Expression*>((*result.top_level_definition)->const_body());
        CHECK(expression->expression_type == ExpressionType::lambda);
        CHECK(expression->type->is_function());
        CHECK(!expression->type->is_pure());
        CHECK(expression->type->arity() == 1);

        auto& body = expression->lambda_value().body;
        CHECK(std::holds_alternative<Statement*>(body));

        auto body_statement = std::get<Statement*>(body);        
        CHECK(body_statement->statement_type == StatementType::block);

        auto block = std::get<Block>(body_statement->value);
        CHECK(block.size() == 2);
        CHECK(block.at(0)->statement_type == StatementType::expression_statement);
        CHECK(block.at(1)->statement_type == StatementType::return_);
    }
}
