#include <sstream>
#include "doctest.h"

#include "mapsc/parser/parser_layer1.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/builtins.hh"

using namespace Maps;

class Layer1tests: public ParserLayer1 {
public:

    Layer1tests(CompilationState* state): ParserLayer1{state} {}

    TEST_CASE_CLASS("simplify_single_statement_block") {
        TypeStore types{};
        CompilationState state{get_builtins(), &types};

        Layer1tests layer1{&state};
        auto ast = state.ast_store_.get();

        Statement* block = ast->allocate_statement({StatementType::block, TEST_SOURCE_LOCATION});
        Statement* inner = ast->allocate_statement({StatementType::expression_statement, TEST_SOURCE_LOCATION});
        
        Expression* value = create_numeric_literal(*ast, "4", TEST_SOURCE_LOCATION);
        inner->value = value;

        std::get<Block>(block->value).push_back(inner);
        Statement inner_copy = *inner;

        auto success = layer1.simplify_single_statement_block(block);

        CHECK(success);
        CHECK(state.is_valid);
        CHECK(block->statement_type == StatementType::expression_statement);
        CHECK(std::get<Expression*>(block->value) == value);
        CHECK(*block == inner_copy);
    };
};

TEST_CASE("layer1 eval should simplify single statement blocks") {
    TypeStore types{};
    CompilationState state{get_builtins(), &types};
    
    ParserLayer1 layer1{&state};

    REQUIRE(state.ast_store_->empty());

    #define CURLY_BRACE_SUBCASE(test_string)\
        SUBCASE(test_string) {\
            std::string source = test_string;\
            std::stringstream source_s{source};\
            \
            auto callable = layer1.eval_parse(source_s);\
            \
            CHECK(state.is_valid);\
            CHECK(callable);\
            CHECK(std::holds_alternative<Expression*>((*callable)->body_));\
            \
            auto expression = std::get<Expression*>((*callable)->body_);\
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
    CompilationState state{get_builtins(), &types};

    ParserLayer1 layer1{&state};

    REQUIRE(state.ast_store_->empty());
    
    SUBCASE("(\"asd\")") {
        auto source = std::stringstream{"(\"asd\")"};
        auto callable = layer1.eval_parse(source);
    
        CHECK(state.is_valid);
        CHECK(callable);
        CHECK(std::holds_alternative<Expression*>((*callable)->body_));
        auto expression = std::get<Expression*>((*callable)->body_);
        CHECK(expression->expression_type == ExpressionType::string_literal);
        CHECK(expression->string_value() == "asd");
    }

    SUBCASE("\"10\" + 5") {
        auto source = std::stringstream{"\"10\" + 5"};
        auto callable = layer1.eval_parse(source);
        
        CHECK(state.is_valid);
        CHECK(callable);
        CHECK(std::holds_alternative<Expression*>((*callable)->body_));
        auto expression = std::get<Expression*>((*callable)->body_);
        
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
        auto callable = layer1.eval_parse(source);

        CHECK(state.is_valid);
        CHECK(callable);
        CHECK(std::holds_alternative<Expression*>((*callable)->body_));
        auto expression = std::get<Expression*>((*callable)->body_);

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
        auto callable = layer1.eval_parse(source);

        CHECK(state.is_valid);
        CHECK(callable);
        CHECK(std::holds_alternative<Expression*>((*callable)->body_));
        auto expression = std::get<Expression*>((*callable)->body_);

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
    auto state = CompilationState{get_builtins(), &types};

    std::stringstream source{"-5"};

    ParserLayer1 layer1{&state};

    auto callable = layer1.eval_parse(source);
    CHECK(state.is_valid);
    CHECK(callable);

    CHECK(std::holds_alternative<Expression*>((*callable)->body_));
    auto expression = std::get<Expression*>((*callable)->body_);

    CHECK(expression->expression_type == ExpressionType::termed_expression);
    auto terms = expression->terms();
    CHECK(terms.size() == 2);
    CHECK(terms.at(0)->expression_type == ExpressionType::minus_sign);
    CHECK(terms.at(1)->expression_type == ExpressionType::numeric_literal);
    CHECK(terms.at(1)->string_value() == "5");
}

TEST_CASE("Should correctly produce empty results") {
    auto [state, _1, _2] = CompilationState::create_test_state();
    REQUIRE(state.empty());
    ParserLayer1 layer1{&state};

    SUBCASE("control") {
        std::stringstream source{"2"};

        layer1.eval_parse(source);
        CHECK(!state.empty());
    }

    SUBCASE("empty string") {
        std::stringstream source{""};

        layer1.eval_parse(source);
        CHECK(state.empty());
    }

    SUBCASE("comment") {
        std::stringstream source{"//comment"};

        layer1.eval_parse(source);
        CHECK(state.empty());
    }
}