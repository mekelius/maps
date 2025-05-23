#include <sstream>
#include "doctest.h"

#include "mapsc/parser/layer1.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/builtins.hh"

using namespace Maps;

TEST_CASE("layer1 eval should not put top level let statements into the root") {
    auto [state, types, builtins] = CompilationState::create_test_state();
    RT_Scope scope{};

    std::stringstream source{"let x = 5"};
    auto [success, top_level_definition, _2, _3, _4, _5] = run_layer1_eval(state, scope, source);

    CHECK(success);
    CHECK(!scope.empty());
    CHECK(!top_level_definition);

    CHECK(scope.identifier_exists("x"));
    auto x = *scope.get_identifier("x");
    CHECK(std::holds_alternative<const Expression*>(x->const_body()));
    auto expression = std::get<const Expression*>(x->const_body());
    CHECK(expression->string_value() == "5");
}

TEST_CASE("layer1 eval should simplify single statement blocks") {
    TypeStore types{};
    RT_Scope scope{};
    CompilationState state{get_builtins(), &types};
    
    REQUIRE(state.ast_store_->empty());

    #define CURLY_BRACE_SUBCASE(test_string)\
        SUBCASE(test_string) {\
            std::string source = test_string;\
            std::stringstream source_s{source};\
            \
            auto [success, definition, _1, _2, _3, _4] = run_layer1_eval(state, scope, source_s);\
            \
            CHECK(success);\
            CHECK(definition);\
            CHECK(std::holds_alternative<const Expression*>((*definition)->const_body()));\
            \
            auto expression = std::get<const Expression*>((*definition)->const_body());\
            \
            CHECK(expression->expression_type == ExpressionType::numeric_literal);\
            CHECK(expression->string_value() == "4");\
        }\

    CURLY_BRACE_SUBCASE("{ 4 }");
    CURLY_BRACE_SUBCASE("{4}");
    CURLY_BRACE_SUBCASE("{{ {4} }}");
}

TEST_CASE("Should handle various cases") {
    TypeStore types{};
    RT_Scope scope{};
    CompilationState state{get_builtins(), &types};

    REQUIRE(state.ast_store_->empty());
    
    SUBCASE("(\"asd\")") {
        auto source = std::stringstream{"(\"asd\")"};
        auto [success, definition, _1, _2, _3, _4] = run_layer1_eval(state, scope, source);
    
        CHECK(success);
        CHECK(definition);
        CHECK(std::holds_alternative<const Expression*>((*definition)->const_body()));
        auto expression = std::get<const Expression*>((*definition)->const_body());
        CHECK(expression->expression_type == ExpressionType::string_literal);
        CHECK(expression->string_value() == "asd");
    }

    SUBCASE("\"10\" + 5") {
        auto source = std::stringstream{"\"10\" + 5"};
        auto [success, definition, _1, _2, _3, _4] = run_layer1_eval(state, scope, source);
        
        CHECK(success);
        CHECK(definition);
        CHECK(std::holds_alternative<const Expression*>((*definition)->const_body()));
        auto expression = std::get<const Expression*>((*definition)->const_body());
        
        CHECK(expression->expression_type == ExpressionType::termed_expression);
        
        auto terms = expression->terms();
        CHECK(terms.size() == 3);
        
        auto lhs = terms.at(0);
        auto op = terms.at(1);
        auto rhs = terms.at(2);

        CHECK((lhs->expression_type == ExpressionType::value || 
            lhs->expression_type == ExpressionType::string_literal));
        CHECK(op->expression_type == ExpressionType::operator_identifier);
        CHECK((rhs->expression_type == ExpressionType::value || 
            rhs->expression_type == ExpressionType::numeric_literal));
    }

    SUBCASE("\"10\"+5") {
        auto source = std::stringstream{"\"10\"+5"};
        auto [success, definition, _1, _2, _3, _4] = run_layer1_eval(state, scope, source);

        CHECK(success);
        CHECK(definition);
        CHECK(std::holds_alternative<const Expression*>((*definition)->const_body()));
        auto expression = std::get<const Expression*>((*definition)->const_body());

        CHECK(expression->expression_type == ExpressionType::termed_expression);
        
        auto terms = expression->terms();
        CHECK(terms.size() == 3);
        
        auto lhs = terms.at(0);
        auto op = terms.at(1);
        auto rhs = terms.at(2);

        CHECK((lhs->expression_type == ExpressionType::value || 
            lhs->expression_type == ExpressionType::string_literal));
        CHECK(op->expression_type == ExpressionType::operator_identifier);
        CHECK((rhs->expression_type == ExpressionType::value || 
            rhs->expression_type == ExpressionType::numeric_literal));
    }

    SUBCASE("(\"10\"+5)") {
        auto source = std::stringstream{"\"10\"+5"};
        auto [success, definition, _1, _2, _3, _4] = run_layer1_eval(state, scope, source);

        CHECK(success);
        CHECK(definition);
        CHECK(std::holds_alternative<const Expression*>((*definition)->const_body()));
        auto expression = std::get<const Expression*>((*definition)->const_body());

        CHECK(expression->expression_type == ExpressionType::termed_expression);
        
        auto terms = expression->terms();
        CHECK(terms.size() == 3);
        
        auto lhs = terms.at(0);
        auto op = terms.at(1);
        auto rhs = terms.at(2);

        CHECK((lhs->expression_type == ExpressionType::value || 
            lhs->expression_type == ExpressionType::string_literal));
        CHECK(op->expression_type == ExpressionType::operator_identifier);
        CHECK((rhs->expression_type == ExpressionType::value || 
            rhs->expression_type == ExpressionType::numeric_literal));
    }
}

TEST_CASE("Should recognize minus as a special case") {
    auto types = TypeStore{};
    RT_Scope scope{};
    auto state = CompilationState{get_builtins(), &types};

    std::stringstream source{"-5"};

    auto [success, definition, _1, _2, _3, _4] = run_layer1_eval(state, scope, source);
    CHECK(success);
    CHECK(definition);

    CHECK(std::holds_alternative<const Expression*>((*definition)->const_body()));
    auto expression = std::get<const Expression*>((*definition)->const_body());

    CHECK(expression->expression_type == ExpressionType::termed_expression);
    auto terms = expression->terms();
    CHECK(terms.size() == 2);
    CHECK(terms.at(0)->expression_type == ExpressionType::minus_sign);
    CHECK(terms.at(1)->expression_type == ExpressionType::numeric_literal);
    CHECK(terms.at(1)->string_value() == "5");
}

TEST_CASE("Should correctly produce empty results") {
    auto [state, _1, _2] = CompilationState::create_test_state();
    RT_Scope scope;
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