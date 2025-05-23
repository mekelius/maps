#include "doctest.h"

#include <sstream>

#include "mapsc/parser/layer1.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Should parse a block") {
    auto [state, types, _] = CompilationState::create_test_state();
    RT_Scope scope{};

    REQUIRE(state.ast_store_->empty());
    REQUIRE(scope.empty());

    SUBCASE("{\n  89;\n  123;\n}") {
        std::stringstream source{"{\n  89;\n  123;\n}"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto body = (*result.top_level_definition)->const_body();
        CHECK(std::holds_alternative<const Statement*>(body));
        
        auto statement = std::get<const Statement*>((*result.top_level_definition)->const_body());
        CHECK(statement->statement_type == StatementType::block);

        auto block = std::get<Block>(statement->value);
        CHECK(block.size() == 2);
        CHECK(block.at(0)->statement_type == StatementType::expression_statement);
        CHECK(block.at(1)->statement_type == StatementType::expression_statement);
    }

    SUBCASE("{\n  89\n  123\n}") {
        std::stringstream source{"{\n  89\n  123\n}"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto body = (*result.top_level_definition)->const_body();
        CHECK(std::holds_alternative<const Statement*>(body));
        
        auto statement = std::get<const Statement*>((*result.top_level_definition)->const_body());
        CHECK(statement->statement_type == StatementType::block);

        auto block = std::get<Block>(statement->value);
        CHECK(block.size() == 2);
        CHECK(block.at(0)->statement_type == StatementType::expression_statement);
        CHECK(block.at(1)->statement_type == StatementType::expression_statement);
    }

    SUBCASE("{ 89; 123; }") {
        std::stringstream source{"{ 89; 123; }"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto body = (*result.top_level_definition)->const_body();
        CHECK(std::holds_alternative<const Statement*>(body));
        
        auto statement = std::get<const Statement*>((*result.top_level_definition)->const_body());
        CHECK(statement->statement_type == StatementType::block);

        auto block = std::get<Block>(statement->value);
        CHECK(block.size() == 2);
        CHECK(block.at(0)->statement_type == StatementType::expression_statement);
        CHECK(block.at(1)->statement_type == StatementType::expression_statement);
    }

    SUBCASE("Statement shouldn't eat the block ender: { print(x); return x + 2; }") {
        std::stringstream source{"{ print(x); return x + 2; }"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto body = (*result.top_level_definition)->const_body();
        CHECK(std::holds_alternative<const Statement*>(body));

        auto statement = std::get<const Statement*>((*result.top_level_definition)->const_body());
        CHECK(statement->statement_type == StatementType::block);

        auto block = std::get<Block>(statement->value);
        CHECK(block.size() == 2);
        CHECK(block.at(0)->statement_type == StatementType::expression_statement);
        CHECK(block.at(1)->statement_type == StatementType::return_);
    }

    SUBCASE("{ 123; 876 }") {
        std::stringstream source{"{ 123; 876 }"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto body = (*result.top_level_definition)->const_body();
        CHECK(std::holds_alternative<const Statement*>(body));

        auto statement = std::get<const Statement*>((*result.top_level_definition)->const_body());
        CHECK(statement->statement_type == StatementType::block);

        auto block = std::get<Block>(statement->value);
        CHECK(block.size() == 2);
        CHECK(block.at(0)->statement_type == StatementType::expression_statement);
        CHECK(block.at(1)->statement_type == StatementType::expression_statement);
    }

    SUBCASE("{ print(x); return x + 2 }") {
        std::stringstream source{"{ print(x); return x + 2 }"};

        auto result = run_layer1_eval(state, scope, source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto body = (*result.top_level_definition)->const_body();
        CHECK(std::holds_alternative<const Statement*>(body));

        auto statement = std::get<const Statement*>((*result.top_level_definition)->const_body());
        CHECK(statement->statement_type == StatementType::block);

        auto block = std::get<Block>(statement->value);
        CHECK(block.size() == 2);
        CHECK(block.at(0)->statement_type == StatementType::expression_statement);
        CHECK(block.at(1)->statement_type == StatementType::return_);
    }
}