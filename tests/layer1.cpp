#include <sstream>
#include "doctest.h"

#include "mapsc/parser/parser_layer1.hh"

using namespace Maps;

class Layer1tests: public ParserLayer1 {
public:

    Layer1tests(auto ast, auto pragmas): ParserLayer1(ast, pragmas) {}

    TEST_CASE_CLASS("simplify_single_statement_block") {
        AST ast;
        PragmaStore pragmas;

        Layer1tests layer1{&ast, &pragmas};

        Statement* block = ast.create_statement(StatementType::block, TEST_SOURCE_LOCATION);
        Statement* inner = ast.create_statement(StatementType::expression_statement, TEST_SOURCE_LOCATION);
        
        Expression* value = ast.create_string_literal("test", TEST_SOURCE_LOCATION);
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

TEST_CASE("layer1 eval should collapse single statement blocks") {
    AST ast;

    CHECK(ast.empty());

    PragmaStore pragmas;

    ParserLayer1 layer1{&ast, &pragmas};

    REQUIRE(ast.empty());
    REQUIRE(pragmas.empty());

    SUBCASE("{{ {4} }}") {
        std::string source = "{{ {4} }}";
        std::stringstream source_s{source};
        
        auto callable = layer1.eval_parse(source_s);
        
        CHECK(ast.is_valid);
        CHECK(callable);
        CHECK(std::holds_alternative<Expression*>((*callable)->body));
        
        auto expression = std::get<Expression*>((*callable)->body);
        
        CHECK(expression->expression_type == ExpressionType::numeric_literal);
        CHECK(expression->string_value() == "4");
    }
}
