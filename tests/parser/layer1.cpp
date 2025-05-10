#include <sstream>
#include "doctest.h"

#include "mapsc/parser/parser_layer1.hh"
#include "mapsc/ast/ast_store.hh"

using namespace Maps;

class Layer1tests: public ParserLayer1 {
public:

    Layer1tests(auto ast, auto pragmas): ParserLayer1(ast, pragmas) {}

    TEST_CASE_CLASS("simplify_single_statement_block") {
        AST_Store ast;
        PragmaStore pragmas;

        Layer1tests layer1{&ast, &pragmas};

        Statement* block = ast.create_statement(StatementType::block, TEST_SOURCE_LOCATION);
        Statement* inner = ast.create_statement(StatementType::expression_statement, TEST_SOURCE_LOCATION);
        
        Expression* value = ast.create_numeric_literal("4", TEST_SOURCE_LOCATION);
        inner->value = value;

        std::get<Block>(block->value).push_back(inner);
        Statement inner_copy = *inner;

        auto success = layer1.simplify_single_statement_block(block);

        CHECK(success);
        CHECK(ast.is_valid);
        CHECK(block->statement_type == StatementType::expression_statement);
        CHECK(std::get<Expression*>(block->value) == value);
        CHECK(*block == inner_copy);
    };

};

TEST_CASE("layer1 eval should simplify single statement blocks") {
    AST_Store ast;
    PragmaStore pragmas;

    CHECK(ast.empty());

    ParserLayer1 layer1{&ast, &pragmas};

    REQUIRE(ast.empty());
    REQUIRE(pragmas.empty());

    #define CURLY_BRACE_SUBCASE(test_string)\
        SUBCASE(test_string) {\
            std::string source = test_string;\
            std::stringstream source_s{source};\
            \
            auto callable = layer1.eval_parse(source_s);\
            \
            CHECK(ast.is_valid);\
            CHECK(callable);\
            CHECK(std::holds_alternative<Expression*>((*callable)->body));\
            \
            auto expression = std::get<Expression*>((*callable)->body);\
            \
            CHECK(expression->expression_type == ExpressionType::numeric_literal);\
            CHECK(expression->string_value() == "4");\
        }\

    CURLY_BRACE_SUBCASE("{ 4 }");
    CURLY_BRACE_SUBCASE("{4}");
    CURLY_BRACE_SUBCASE("{{ {4} }}");
}

TEST_CASE("Should handle various cases") {
    AST_Store ast;
    PragmaStore pragmas;

    REQUIRE(ast.empty());
    ParserLayer1 layer1{&ast, &pragmas};
    
    SUBCASE("(\"asd\")") {
        auto source = std::stringstream{"(\"asd\")"};
        auto callable = layer1.eval_parse(source);
    
        CHECK(ast.is_valid);
        CHECK(callable);
        CHECK(std::holds_alternative<Expression*>((*callable)->body));
        auto expression = std::get<Expression*>((*callable)->body);
        CHECK(expression->expression_type == ExpressionType::string_literal);
        CHECK(expression->string_value() == "asd");
    }

    SUBCASE("\"10\" + 5") {
        auto source = std::stringstream{"\"10\" + 5"};
        auto callable = layer1.eval_parse(source);
        
        CHECK(ast.is_valid);
        CHECK(callable);
        CHECK(std::holds_alternative<Expression*>((*callable)->body));
        auto expression = std::get<Expression*>((*callable)->body);
        
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

        CHECK(ast.is_valid);
        CHECK(callable);
        CHECK(std::holds_alternative<Expression*>((*callable)->body));
        auto expression = std::get<Expression*>((*callable)->body);

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

        CHECK(ast.is_valid);
        CHECK(callable);
        CHECK(std::holds_alternative<Expression*>((*callable)->body));
        auto expression = std::get<Expression*>((*callable)->body);

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
