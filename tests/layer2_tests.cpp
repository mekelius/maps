#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "../src/lang/ast.hh"
#include "../src/parsing/parser_layer2.hh"

using AST::Expression, AST::ExpressionType;

TEST_CASE("TermedExpressionParser should replace a single value term with that value") {
    AST::AST ast{};

    Expression* expr = ast.create_expression(ExpressionType::termed_expression, {0,0});
    Expression* value = ast.create_expression(ExpressionType::string_literal, {0,0});
    value->value = "TEST_STRING:oasrpkorsapok";
    expr->terms().push_back(value);

    TermedExpressionParser parser{&ast, expr};
    parser.run();

    CHECK(expr->expression_type == ExpressionType::string_literal);
    CHECK(expr->string_value() == value->string_value());
}

