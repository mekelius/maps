#include "doctest.h"

#include <string>
#include <sstream>

#include "mapsc/types/type.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/parser/parser_layer2.hh"

using namespace Maps;

TEST_CASE("Should parse a numberliteral with the correct type") {
    AST_Store ast{};
    auto expr = create_termed_expression(ast, {
        create_type_reference(ast, &Int, {0,0}),
        create_numeric_literal(ast, "23", {0,0})
    }, {0,0});

    TermedExpressionParser{&ast, expr}.run();

    CHECK(*expr->type == Int);
}

TEST_CASE("Should be able to cast a string literal into Int") {
    AST_Store ast{};
    
    auto expr = create_termed_expression(ast, {
        create_type_reference(ast, &Int, {0,0}),
        create_string_literal(ast, "23", {0,0})
    }, {0,0});

    TermedExpressionParser{&ast, expr}.run();

    CHECK(*expr->type == Int);
    CHECK(std::get<maps_Int>(expr->value) == 23);
}
