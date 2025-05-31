#include "doctest.h"

#include <sstream>

#include "mapsc/parser/layer1.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;

TEST_CASE("Should parse lambdas") {
    auto [state, types, _] = CompilationState::create_test_state();
    Scope scope{};

    REQUIRE(state.ast_store_->empty());
    REQUIRE(scope.empty());

    SUBCASE("\\x -> x") {
        std::stringstream source{"\\x -> x"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto expression = std::get<const Expression*>((*result.top_level_definition)->const_body());
        CHECK(expression->expression_type == ExpressionType::reference);
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
        CHECK(expression->expression_type == ExpressionType::reference);
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
        CHECK(expression->expression_type == ExpressionType::reference);
        CHECK(expression->type->is_function());
        CHECK(expression->type->is_pure());
        CHECK(expression->type->arity() == 2);

        auto body = expression->reference_value()->const_body();
        CHECK(std::holds_alternative<const Expression*>(body));

        auto body_expression = std::get<const Expression*>(body);
        CHECK(body_expression->expression_type == ExpressionType::layer2_expression);
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
        CHECK(expression->expression_type == ExpressionType::reference);
        CHECK(expression->type->is_function());
        CHECK(!expression->type->is_pure());
        CHECK(expression->type->arity() == 1);

        auto body = expression->reference_value()->const_body();
        CHECK(std::holds_alternative<const Statement*>(body));

        auto body_statement = std::get<const Statement*>(body);        
        CHECK(body_statement->statement_type == StatementType::block);

        StatementValue value = body_statement->value;
        auto block = std::get<Block>(value);
        CHECK(block.size() == 2);
        CHECK(block.at(0)->statement_type == StatementType::expression_statement);
        CHECK(block.at(1)->statement_type == StatementType::return_);
    }
}
