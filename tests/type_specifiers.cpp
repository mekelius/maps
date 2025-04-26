#include "doctest.h"

#include <string>
#include <sstream>

#include "../src/lang/type.hh"
#include "../src/lang/ast.hh"
#include "../src/parsing/full_parse.hh"
#include "../src/parsing/parser_layer2.hh"

using namespace Maps;


TEST_CASE("Should parse a numberliteral with the correct type") {
    AST ast{};
    auto expr = ast.create_termed_expression({
        ast.create_type_reference(&Int, {0,0}),
        ast.create_numeric_literal("23", {0,0})
    }, {0,0});
    ast.globals_->create_callable("x", expr);

    TermedExpressionParser{&ast, expr}.run();

    CHECK(expr->declared_type);
    CHECK(**expr->declared_type == Int);
}

TEST_CASE("Integration test: parse_source should parse a numberliteral with the correct type") {
    std::stringstream source{"let x = Int 34"};
    
    auto result = parse_source(source, false);

    CHECK(result);

    auto [ast, _1] = std::move(*result);

    CHECK(ast->globals_->identifier_exists("x"));

    auto x = (*ast->globals_->get_identifier("x"));

    CHECK(x->declared_type);
    CHECK(**x->declared_type == Int);
    CHECK(*x->get_type() == Int);
}