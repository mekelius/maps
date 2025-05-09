#include "doctest.h"

#include <sstream>
#include <iostream>

#include "mapsc/ast/ast_store.hh"
#include "mapsc/parser/parser_layer2.hh"

using namespace Maps;

// --------------- HELPERS ---------------

inline std::tuple<Expression*, Callable*> create_operator_helper(AST_Store& ast, 
    const std::string& op_string, unsigned int precedence = 500) {

    const Type* type = ast.types_->get_function_type(Void, {&Number, &Number});
    Callable* op_callable = ast.create_builtin_binary_operator(op_string, *type, precedence);

    Expression* op_ref = ast.create_operator_ref(op_callable, {0,0});

    return {op_ref, op_callable};
}

// takes a parsed binop expression tree and traverses it in preorder
void traverse_pre_order(Expression* tree, std::ostream& output) {
    auto [op, args] = tree->call_value();

    Expression* lhs = args.at(0);
    Expression* rhs = args.at(1);

    output << op->name;

    if (lhs->expression_type == ExpressionType::call) {
        traverse_pre_order(lhs, output);
    } else {
        output << lhs->string_value();
    }

    if (rhs->expression_type == ExpressionType::call) {
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
    AST_Store ast{};

    Expression* expr = ast.create_termed_expression({}, {0,0});

    REQUIRE(expr->terms().size() == 0);

    SUBCASE("String value") {
        Expression* value = ast.create_string_literal("TEST_STRING:oasrpkorsapok", {0,0});
        expr->terms().push_back(value);
        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::string_literal);
        CHECK(expr->string_value() == value->string_value());
    }

    SUBCASE("Number value") {
        Expression* value = ast.create_numeric_literal("234.52", {0,0});
        expr->terms().push_back(value);
        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::numeric_literal);
        CHECK(expr->string_value() == value->string_value());
    }
}

TEST_CASE("TermedExpressionParser should handle binop expressions") {
    AST_Store ast{};
    Expression* expr = ast.create_termed_expression({}, {0,0});

    Expression* val1 = ast.create_numeric_literal("23", {0,0});
    Expression* val2 = ast.create_numeric_literal("12", {0,0});

    // create the operator
    auto [op1_ref, op1] = create_operator_helper(ast, "+");

    expr->terms() = std::vector<Expression*>{val1, op1_ref, val2};

    REQUIRE(expr->terms().size() == 3);

    SUBCASE("Simple expression") {
        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::call);
        // NOTE: the operator_ref should be unwrapped

        auto [op, args] = expr->call_value();
        CHECK(op == op1);
        CHECK(args.at(0) == val1);
        CHECK(args.at(1) == val2);
    }

    SUBCASE("should handle higher precedence first") {
        // create operator with lower precedence
        auto [op2_ref, op2] = create_operator_helper(ast, "*", 1);
        expr->terms().push_back(op2_ref);

        Expression* val3 = ast.create_numeric_literal("235", {0,0});
        expr->terms().push_back(val3);

        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::call);
        auto [outer_op, outer_args] = expr->call_value();

        auto outer_lhs = outer_args.at(0);
        auto outer_rhs = outer_args.at(1);

        CHECK(outer_op == op2);
        CHECK(outer_rhs == val3);

        // check the lhs
        CHECK(outer_lhs->expression_type == ExpressionType::call);
        auto [inner_op, inner_args] = outer_lhs->call_value();

        auto inner_lhs = inner_args.at(0);
        auto inner_rhs = inner_args.at(1);

        CHECK(inner_op == op1);
        CHECK(inner_lhs == val1);
        CHECK(inner_rhs == val2);
    }

    SUBCASE("should handle lower precedence first") {
        // create operator with lower precedence
        auto [op2_ref, op2] = create_operator_helper(ast, "*", 999);
        expr->terms().push_back(op2_ref);

        Expression* val3 = ast.create_numeric_literal("235", {0,0});

        expr->terms().push_back(val3);

        TermedExpressionParser{&ast, expr}.run();

        CHECK(expr->expression_type == ExpressionType::call);
        auto [outer_op, args] = expr->call_value();

        auto outer_lhs = args.at(0);
        auto outer_rhs = args.at(1);

        CHECK(outer_op == op1);
        CHECK(outer_lhs == val1);

        // check the lhs
        CHECK(outer_rhs->expression_type == ExpressionType::call);
        auto [inner_op, inner_args] = outer_rhs->call_value();
        auto inner_lhs = inner_args.at(0);
        auto inner_rhs = inner_args.at(1);

        CHECK(inner_op == op2);
        CHECK(inner_lhs == val2);
        CHECK(inner_rhs == val3);
    }
}

TEST_CASE ("should handle more complex expressions") {
    AST_Store ast{};
    Expression* expr = ast.create_termed_expression({}, {0,0});

    auto [op1_ref, op1] = create_operator_helper(ast, "1", 1);
    auto [op2_ref, op2] = create_operator_helper(ast, "2", 2);
    auto [op3_ref, op3] = create_operator_helper(ast, "3", 3);
    Expression* val = ast.create_numeric_literal("v", {0,0});

    REQUIRE(expr->terms().size() == 0);

    SUBCASE("1 2 1") {
        std::string input = "1 2 1";
        std::string expected_pre_order = "11v2vvv";
        
        prime_terms(expr, input, op1_ref, op2_ref, op3_ref, val);

        TermedExpressionParser{&ast, expr}.run();
        
        std::stringstream output;
        traverse_pre_order(expr, output);
        CHECK(output.str() == expected_pre_order);
    }

    SUBCASE("1 2 2 2 1 2 2 2 3 3 2 2 1") {
        std::string input = "1 2 2 2 1 2 2 2 3 3 2 2 1";
        std::string expected_pre_order = "111v222vvvv22222vvv33vvvvvv";
        
        prime_terms(expr, input, op1_ref, op2_ref, op3_ref, val);

        TermedExpressionParser{&ast, expr}.run();
        
        std::stringstream output;
        traverse_pre_order(expr, output);
        CHECK(output.str() == expected_pre_order);
    }
}

TEST_CASE("TermedExpressionParser should handle haskell-style call expressions") {
    AST_Store ast{};
    Expression* expr = ast.create_termed_expression({}, {0,0});
    
    SUBCASE("1 arg") {    
        const Type* function_type = ast.types_->get_function_type(Void, {&String});

        Callable* function = ast.create_builtin("test_f", *function_type);
        Expression* id = ast.globals_->create_reference_expression(function, {0,0});
        id->value = function;    
        id->type = function_type;

        Expression* arg1 = ast.create_string_literal("", {0,0});


        expr->terms().push_back(id);
        expr->terms().push_back(arg1);

        TermedExpressionParser{&ast, expr}.run();
        
        CHECK(expr->expression_type == ExpressionType::call);
        auto [callee, args] = expr->call_value();
        CHECK(callee->name == "test_f");
        CHECK(args.size() == 1);
        CHECK(args.at(0) == arg1);
    }

    SUBCASE("4 args") {    
        const Type* function_type = ast.types_->get_function_type(Void, 
            {&String, &String, &String, &String});
        
        Callable* function = ast.create_builtin("test_f", *function_type);
        Expression* id = ast.globals_->create_reference_expression(function, {0,0});
        id->value = function;    
        id->type = function_type;
    
        Expression* arg1 = ast.create_string_literal("", {0,0});
        Expression* arg2 = ast.create_string_literal("", {0,0});
        Expression* arg3 = ast.create_string_literal("", {0,0});
        Expression* arg4 = ast.create_string_literal("", {0,0});

        expr->terms().push_back(id);
        expr->terms().push_back(arg1);
        expr->terms().push_back(arg2);
        expr->terms().push_back(arg3);
        expr->terms().push_back(arg4);

        TermedExpressionParser{&ast, expr}.run();
        
        CHECK(expr->expression_type == ExpressionType::call);
        auto [callee, args] = expr->call_value();
        CHECK(callee->name == "test_f");
        CHECK(args.size() == 4);
        CHECK(args.at(0) == arg1);
        CHECK(args.at(1) == arg2);
        CHECK(args.at(2) == arg3);
        CHECK(args.at(3) == arg4);
    }

    SUBCASE("If the call is not partial, the call expression's type should be the return type") {

        const Type* function_type = ast.types_->get_function_type(Number, {&String});
        
        Callable* function = ast.create_builtin("test_f", *function_type);
        Expression* id = ast.globals_->create_reference_expression(function, {0,0});
        id->value = function;
        id->type = function_type;

        Expression* arg1 = ast.create_string_literal("", {0,0});

        expr->terms().push_back(id);
        expr->terms().push_back(arg1);

        TermedExpressionParser{&ast, expr}.run();
        
        CHECK(expr->type == &Number);
    }
}

TEST_CASE("Should handle partial application of binary operators") {
    AST_Store ast{};

    auto [op_ref, op] = create_operator_helper(ast, "-");
    auto val = ast.create_numeric_literal("34", {0,0});

    SUBCASE("right") {
        Expression* expr = ast.create_termed_expression({op_ref, val}, {0,0});
        
        TermedExpressionParser{&ast, expr}.run();
        
        auto [callee, args] = expr->call_value();
        CHECK(callee == op);
        CHECK(args.size() == 2);
        CHECK(args.at(0)->expression_type == ExpressionType::missing_arg);
        CHECK(args.at(1) == val);
    }

    SUBCASE("left") {
        Expression* expr = ast.create_termed_expression({val, op_ref}, {0,0});
        
        TermedExpressionParser{&ast, expr}.run();
        
        auto [callee, args] = expr->call_value();
        CHECK(callee == op);
        CHECK(args.size() == 2);
        CHECK(args.at(0) == val);
    }
}

TEST_CASE("Should set the type on a non-partial call expression to the return type") {
    AST_Store ast{};

    const FunctionType* IntString = ast.types_->get_function_type(String, {&Int}, false);

    auto test_f_expr = Expression{ExpressionType::value, TSL, "qwe", IntString};
    auto test_f = ast.globals_->create_callable("test_f", &test_f_expr, TSL);
    REQUIRE(test_f);

    auto arg = Expression{ExpressionType::numeric_literal, TSL, "3", &NumberLiteral};
    auto reference = ast.globals_->create_reference_expression(*test_f, TSL);

    auto expr = ast.create_termed_expression({reference, &arg}, TSL);

    TermedExpressionParser{&ast, expr}.run();

    CHECK(ast.is_valid);
    CHECK(expr->expression_type == ExpressionType::call);
    CHECK(*expr->type == String);
    CHECK(expr->call_value() == CallExpressionValue{*test_f, {&arg}});
}


TEST_CASE("Should set the type on a non-partial \"operator expression\" to the return type") {
    AST_Store ast{};

    const FunctionType* IntString = ast.types_->get_function_type(String, {&Int, &Int}, false);

    auto test_op_expr = Expression{ExpressionType::value, TSL, "jii", IntString};
    auto test_op = ast.globals_->create_binary_operator(">=?", &test_op_expr, 5, 
        Associativity::left, TSL);
    REQUIRE(test_op);    

    auto lhs = Expression{ExpressionType::numeric_literal, TSL, "3", &NumberLiteral};
    auto rhs = Expression{ExpressionType::numeric_literal, TSL, "7", &NumberLiteral};

    auto reference = ast.create_operator_ref(*test_op, TSL);

    auto expr = ast.create_termed_expression({&lhs, reference, &rhs}, TSL);

    TermedExpressionParser{&ast, expr}.run();

    CHECK(ast.is_valid);
    CHECK(expr->expression_type == ExpressionType::call);
    CHECK(expr->call_value() == CallExpressionValue{*test_op, {&lhs, &rhs}});
    std::cout << expr->type->to_string();
    CHECK(*expr->type == String);
}