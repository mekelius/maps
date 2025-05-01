#include "doctest.h"

#include <string>
#include <sstream>

#include "mapsc/types/type.hh"
#include "mapsc/ast/ast.hh"
#include "mapsc/parser/full_parse.hh"
#include "mapsc/parser/parser_layer2.hh"

using namespace Maps;


TEST_CASE("Should parse a numberliteral with the correct type") {
    AST ast{};
    auto expr = ast.create_termed_expression({
        ast.create_type_reference(&Int, {0,0}),
        ast.create_numeric_literal("23", {0,0})
    }, {0,0});

    TermedExpressionParser{&ast, expr}.run();

    CHECK(*expr->type == Int);
}

TEST_CASE("Should be able to cast a string literal into Int") {
    AST ast{};
    
    auto expr = ast.create_termed_expression({
        ast.create_type_reference(&Int, {0,0}),
        ast.create_string_literal("23", {0,0})
    }, {0,0});

    TermedExpressionParser{&ast, expr}.run();

    CHECK(*expr->type == Int);
    CHECK(std::get<int>(expr->value) == 23);
}

TEST_CASE("Integration test: parse_source should parse a numberliteral with the correct type") {
    std::stringstream source{"let x = Int 34"};
    
    auto [success, ast, _1] = parse_source(source);

    CHECK(success);

    CHECK(ast->globals_->identifier_exists("x"));

    auto x = (*ast->globals_->get_identifier("x"));

    CHECK(*x->get_type() == Int);
}
