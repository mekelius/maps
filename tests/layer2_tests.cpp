#include "doctest.h"

#include <sstream>

#include "../src/lang/ast.hh"
#include "../src/parsing/parser_layer2.hh"

using AST::Expression, AST::ExpressionType, AST::Callable;

// --------------- HELPERS ---------------

// jesus christ how horrifying
inline std::tuple<AST::Expression*, AST::Callable*> create_operator_helper(AST::AST& ast, 
    const std::string& op_string, unsigned int precedence = 500) {
    AST::Expression* op = ast.create_expression(AST::ExpressionType::builtin_operator, {0,0});
    op->value = op_string;
    op->type = AST::create_binary_operator_type(AST::Void, AST::Number, AST::Number, precedence);
    AST::Callable* op_callable = ast.create_callable({0,0});
    op_callable->body = op;
    AST::Expression* op_ref = ast.create_expression(AST::ExpressionType::operator_ref, {0,0});
    op_ref->value = op_callable;
    op_ref->type = op_callable->get_type();

    return {op_ref, op_callable};
}

// takes a parsed binop expression tree and traverses it in preorder
void traverse_pre_order(AST::Expression* tree, std::ostream& output) {
    auto [op, lhs, rhs] = tree->binop_apply_value();

    output << std::get<AST::Expression*>(op->body)->string_value();

    if (lhs->expression_type == AST::ExpressionType::binary_operator_apply) {
        traverse_pre_order(lhs, output);
    } else {
        output << lhs->string_value();
    }

    if (rhs->expression_type == AST::ExpressionType::binary_operator_apply) {
        traverse_pre_order(rhs, output);
    } else {
        output << rhs->string_value();
    }
}

void prime_terms(auto expr, const std::string& input, auto op1_ref, auto op2_ref, auto op3_ref, auto val) {
    expr->terms().push_back(val);
    for (char c: input) {
        switch (c) {
            case ' ':
                break;
            case '1':
                expr->terms().push_back(op1_ref);
                expr->terms().push_back(val);
                break;
            case '2':
                expr->terms().push_back(op2_ref);
                expr->terms().push_back(val);
                break;
            case '3':
                expr->terms().push_back(op3_ref);
                expr->terms().push_back(val);
                break;
        }
    }
}

// --------------- TEST CASES ---------------

TEST_CASE("TermedExpressionParser should replace a single value term with that value") {
    AST::AST ast{};

    Expression* expr = ast.create_expression(ExpressionType::termed_expression, {0,0});

    REQUIRE(expr->terms().size() == 0);

    SUBCASE("String value") {
        Expression* value = ast.create_expression(ExpressionType::string_literal, {0,0});
        value->value = "TEST_STRING:oasrpkorsapok";
        expr->terms().push_back(value);
        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::string_literal);
        CHECK(expr->string_value() == value->string_value());
    }

    SUBCASE("Number value") {
        Expression* value = ast.create_expression(ExpressionType::numeric_literal, {0,0});
        value->value = "234.52";
        expr->terms().push_back(value);
        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::numeric_literal);
        CHECK(expr->string_value() == value->string_value());
    }
}

TEST_CASE("TermedExpressionParser should replace an empty termed expression with ExpressionType::empty") {
    AST::AST ast{};
    Expression* expr = ast.create_expression(ExpressionType::termed_expression, {0,0});
    TermedExpressionParser{&ast, expr}.run();

    CHECK(expr->expression_type == ExpressionType::empty);
}

TEST_CASE("TermedExpressionParser should handle binop expressions") {
    AST::AST ast{};
    Expression* expr = ast.create_expression(ExpressionType::termed_expression, {0,0});

    Expression* val1 = ast.create_expression(ExpressionType::numeric_literal, {0,0});
    val1->value = "23";
    Expression* val2 = ast.create_expression(ExpressionType::numeric_literal, {0,0});
    val2->value = "12";

    // create the operator
    auto [op1_ref, op1] = create_operator_helper(ast, "+");

    expr->value = std::vector<Expression*>{val1, op1_ref, val2};

    REQUIRE(expr->terms().size() == 3);

    SUBCASE("Simple expression") {
        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::binary_operator_apply);
        // NOTE: the operator_ref should be unwrapped
        CHECK(std::tie(op1, val1, val2) == expr->binop_apply_value());
    }

    SUBCASE("should handle higher precedence first") {
        // create operator with lower precedence
        auto [op2_ref, op2] = create_operator_helper(ast, "*", 1);
        expr->terms().push_back(op2_ref);

        Expression* val3 = ast.create_expression(ExpressionType::numeric_literal, {0,0});
        val3->value = "235";
        expr->terms().push_back(val3);

        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::binary_operator_apply);
        auto [outer_op, outer_lhs, outer_rhs] = expr->binop_apply_value();

        CHECK(outer_op == op2);
        CHECK(outer_rhs == val3);

        // check the lhs
        CHECK(outer_lhs->expression_type == ExpressionType::binary_operator_apply);
        auto [inner_op, inner_lhs, inner_rhs] = outer_lhs->binop_apply_value();

        CHECK(inner_op == op1);
        CHECK(inner_lhs == val1);
        CHECK(inner_rhs == val2);
    }

    SUBCASE("should handle lower precedence first") {
        // create operator with lower precedence
        auto [op2_ref, op2] = create_operator_helper(ast, "*", 999);
        expr->terms().push_back(op2_ref);

        Expression* val3 = ast.create_expression(ExpressionType::numeric_literal, {0,0});
        val3->value = "235";
        expr->terms().push_back(val3);

        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::binary_operator_apply);
        auto [outer_op, outer_lhs, outer_rhs] = expr->binop_apply_value();

        CHECK(outer_op == op1);
        CHECK(outer_lhs == val1);

        // check the lhs
        CHECK(outer_rhs->expression_type == ExpressionType::binary_operator_apply);
        auto [inner_op, inner_lhs, inner_rhs] = outer_rhs->binop_apply_value();

        CHECK(inner_op == op2);
        CHECK(inner_lhs == val2);
        CHECK(inner_rhs == val3);
    }
}

TEST_CASE ("should handle more complex expressions") {
    AST::AST ast{};
    Expression* expr = ast.create_expression(ExpressionType::termed_expression, {0,0});

    auto [op1_ref, op1] = create_operator_helper(ast, "1", 1);
    auto [op2_ref, op2] = create_operator_helper(ast, "2", 2);
    auto [op3_ref, op3] = create_operator_helper(ast, "3", 3);
    Expression* val = ast.create_expression(ExpressionType::numeric_literal, {0,0});
    val->value = "v";

    REQUIRE(expr->terms().size() == 0);

    SUBCASE("1 2 1") {
        std::string input = "1 2 1";
        auto expected_pre_order = "11v2vvv";
        
        prime_terms(expr, input, op1_ref, op2_ref, op3_ref, val);

        TermedExpressionParser{&ast, expr}.run();
        
        std::stringstream output;
        traverse_pre_order(expr, output);
        CHECK(output.str() == expected_pre_order);
    }

    SUBCASE("1 2 2 2 1 2 2 2 3 3 2 2 1") {
        std::string input = "1 2 2 2 1 2 2 2 3 3 2 2 1";
        auto expected_pre_order = "111v222vvvv22222vvv33vvvvvv";
        
        prime_terms(expr, input, op1_ref, op2_ref, op3_ref, val);

        TermedExpressionParser{&ast, expr}.run();
        
        std::stringstream output;
        traverse_pre_order(expr, output);
        CHECK(output.str() == expected_pre_order);
    }
}

TEST_CASE("TermedExpressionParser should handle haskell-style call expressions") {
    AST::AST ast{};
    Expression* expr = ast.create_expression(ExpressionType::termed_expression, {0,0});
    
    Expression* function = ast.create_expression(ExpressionType::builtin_function, {0,0});
    ast.globals_.create_identifier("test_f", function, {0,0});
    Expression* id = ast.create_expression(ExpressionType::identifier, {0,0});
    id->value = "test_f";
    
    SUBCASE("1 arg") {    
        function->type = AST::create_function_type(AST::Void, {AST::String});
        id->type = function->type;
        Expression* arg1 = ast.create_expression(ExpressionType::string_literal, {0,0});

        expr->terms().push_back(id);
        expr->terms().push_back(arg1);

        TermedExpressionParser{&ast, expr}.run();
        
        CHECK(expr->expression_type == ExpressionType::call);
        auto [callee, args] = expr->call();
        CHECK(callee == "test_f");
        CHECK(args.size() == 1);
        CHECK(args.at(0) == arg1);
    }

    SUBCASE("4 args") {    
        function->type = AST::create_function_type(AST::Void, 
            {AST::String, AST::String, AST::String, AST::String});
        id->type = function->type;
        Expression* arg1 = ast.create_expression(ExpressionType::string_literal, {0,0});
        Expression* arg2 = ast.create_expression(ExpressionType::string_literal, {0,0});
        Expression* arg3 = ast.create_expression(ExpressionType::string_literal, {0,0});
        Expression* arg4 = ast.create_expression(ExpressionType::string_literal, {0,0});

        expr->terms().push_back(id);
        expr->terms().push_back(arg1);
        expr->terms().push_back(arg2);
        expr->terms().push_back(arg3);
        expr->terms().push_back(arg4);

        TermedExpressionParser{&ast, expr}.run();
        
        CHECK(expr->expression_type == ExpressionType::call);
        auto [callee, args] = expr->call();
        CHECK(callee == "test_f");
        CHECK(args.size() == 4);
        CHECK(args.at(0) == arg1);
        CHECK(args.at(1) == arg2);
        CHECK(args.at(2) == arg3);
        CHECK(args.at(3) == arg4);
    }

    SUBCASE("If the call is not partial, the call expression's type should be the return type") {

        function->type = AST::create_function_type(AST::Number, {AST::String});
        id->type = function->type;
        Expression* arg1 = ast.create_expression(ExpressionType::string_literal, {0,0});

        expr->terms().push_back(id);
        expr->terms().push_back(arg1);

        TermedExpressionParser{&ast, expr}.run();
        
        CHECK(expr->type == AST::Number);
    }
}