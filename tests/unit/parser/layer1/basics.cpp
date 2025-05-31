#include "doctest.h"

#include <sstream>
#include <tuple>

#include "mapsc/parser/layer1.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;
using namespace std;

tuple<CompilationState, Scope, stringstream> setup(const string& source_str) { 
    auto [state, _1, _2] = CompilationState::create_test_state();

    return {
        std::move(state), 
        Scope{}, 
        stringstream{source_str}
    };
}

TEST_CASE("Should handle various cases") {
    TypeStore types{};
    Scope scope{};
    CompilationState state{get_builtins(), &types};

    REQUIRE(state.ast_store_->empty());
    
    SUBCASE("(\"asd\")") {
        auto source = std::stringstream{"(\"asd\")"};
        auto [success, definition, _1, _2, _3, _4] = run_layer1_eval(state, scope, source);
    
        CHECK(success);
        CHECK(definition);
        CHECK(std::holds_alternative<Expression*>((*definition)->value_));
        auto expression = std::get<Expression*>((*definition)->value_);
        CHECK(expression->expression_type == ExpressionType::known_value);
        CHECK(expression->string_value() == "asd");
    }

    SUBCASE("\"10\" + 5") {
        auto source = std::stringstream{"\"10\" + 5"};
        auto [success, definition, _1, _2, _3, _4] = run_layer1_eval(state, scope, source);
        
        CHECK(success);
        CHECK(definition);
        CHECK(std::holds_alternative<Expression*>((*definition)->value_));
        auto expression = std::get<Expression*>((*definition)->value_);
        
        CHECK(expression->expression_type == ExpressionType::layer2_expression);
        
        auto terms = expression->terms();
        CHECK(terms.size() == 3);
        
        auto lhs = terms.at(0);
        auto op = terms.at(1);
        auto rhs = terms.at(2);

        CHECK((lhs->expression_type == ExpressionType::known_value || 
            lhs->expression_type == ExpressionType::known_value));
        CHECK(op->expression_type == ExpressionType::operator_identifier);
        CHECK((rhs->expression_type == ExpressionType::known_value || 
            rhs->expression_type == ExpressionType::known_value));
    }

    SUBCASE("\"10\"+5") {
        auto source = std::stringstream{"\"10\"+5"};
        auto [success, definition, _1, _2, _3, _4] = run_layer1_eval(state, scope, source);

        CHECK(success);
        CHECK(definition);
        CHECK(std::holds_alternative<Expression*>((*definition)->value_));
        auto expression = std::get<Expression*>((*definition)->value_);

        CHECK(expression->expression_type == ExpressionType::layer2_expression);
        
        auto terms = expression->terms();
        CHECK(terms.size() == 3);
        
        auto lhs = terms.at(0);
        auto op = terms.at(1);
        auto rhs = terms.at(2);

        CHECK((lhs->expression_type == ExpressionType::known_value || 
            lhs->expression_type == ExpressionType::known_value));
        CHECK(op->expression_type == ExpressionType::operator_identifier);
        CHECK((rhs->expression_type == ExpressionType::known_value || 
            rhs->expression_type == ExpressionType::known_value));
    }

    SUBCASE("(\"10\"+5)") {
        auto source = std::stringstream{"\"10\"+5"};
        auto [success, definition, _1, _2, _3, _4] = run_layer1_eval(state, scope, source);

        CHECK(success);
        CHECK(definition);
        CHECK(std::holds_alternative<Expression*>((*definition)->value_));
        auto expression = std::get<Expression*>((*definition)->value_);

        CHECK(expression->expression_type == ExpressionType::layer2_expression);
        
        auto terms = expression->terms();
        CHECK(terms.size() == 3);
        
        auto lhs = terms.at(0);
        auto op = terms.at(1);
        auto rhs = terms.at(2);

        CHECK((lhs->expression_type == ExpressionType::known_value || 
            lhs->expression_type == ExpressionType::known_value));
        CHECK(op->expression_type == ExpressionType::operator_identifier);
        CHECK((rhs->expression_type == ExpressionType::known_value || 
            rhs->expression_type == ExpressionType::known_value));
    }
}

TEST_CASE("layer1 eval should not put top level let statements into the root") {
    auto [state, types, builtins] = CompilationState::create_test_state();
    Scope scope{};

    std::stringstream source{"let x = 5"};
    auto [success, top_level_definition, _2, _3, _4, _5] = run_layer1_eval(state, scope, source);

    CHECK(success);
    CHECK(!scope.empty());
    CHECK(!top_level_definition);

    CHECK(scope.identifier_exists("x"));
    auto x = *scope.get_identifier("x");
    CHECK(std::holds_alternative<Expression*>((*x->body_)->value_));
    auto expression = std::get<Expression*>((*x->body_)->value_);
    CHECK(expression->string_value() == "5");
}

TEST_CASE("Should correctly produce empty results") {
    auto [state, _1, _2] = CompilationState::create_test_state();
    Scope scope;
    REQUIRE(scope.empty());

    SUBCASE("control") {
        std::stringstream source{"2"};

        auto result = run_layer1_eval(state, scope, source);
        CHECK(scope.empty());
        CHECK(result.top_level_definition);
    }

    SUBCASE("empty string") {
        std::stringstream source{""};

        auto result = run_layer1_eval(state, scope, source);
        CHECK(!result.top_level_definition);
        CHECK(scope.empty());
    }

    SUBCASE("comment") {
        std::stringstream source{"//comment"};

        auto result = run_layer1_eval(state, scope, source);
        CHECK(scope.empty());
        CHECK(!result.top_level_definition);
    }
}

// TEST_CASE("Should mark context on termed expressions") {
//     auto [state, scope, source] = setup("let f = x + 2");

//     auto result = run_layer1_eval(state, scope, source);

//     CHECK(result.success);
//     CHECK(scope.identifier_exists("f"));

//     auto f = *scope.get_identifier("f");

//     auto expr = std::get<Expression*>(*f->get_body_value());
//     CHECK(expr->expression_type == ExpressionType::layer2_expression);

//     CHECK(false);
//     // CHECK(*expr->termed_context() == *f);
// }

// TEST_CASE("Should mark context on top level termed expressions") {
//     auto [state, scope, source] = setup("x + 2");

//     auto result = run_layer1_eval(state, scope, source);

//     CHECK(result.success);
//     CHECK(result.top_level_definition);

//     auto expr = std::get<Expression*>((*result.top_level_definition)->value_);
//     CHECK(expr->expression_type == ExpressionType::layer2_expression);

//     CHECK(false);
//     // CHECK(*expr->termed_context() == **result.top_level_definition);
// }