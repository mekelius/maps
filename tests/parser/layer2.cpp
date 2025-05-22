#include "doctest.h"

#include <sstream>

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/parser/parser_layer2.hh"

using namespace Maps;
using namespace std;

// --------------- HELPERS ---------------

inline tuple<Expression*, Definition*> create_operator_helper(CompilationState& state, 
    const string& op_string, unsigned int precedence = 500) {

    const Type* type = state.types_->get_function_type(&Void, {&Number, &Number}, true);
    Definition* op_definition = state.ast_store_->allocate_operator(
        RT_Operator::create_binary(op_string, External{}, type, precedence, 
            RT_Operator::Associativity::left, TSL));

    Expression* op_ref = Expression::operator_reference(*state.ast_store_, op_definition, {0,0});

    return {op_ref, op_definition};
}

// takes a parsed binop expression tree and traverses it in preorder
void traverse_pre_order(Expression* tree, ostream& output) {
    auto [op, args] = tree->call_value();

    Expression* lhs = args.at(0);
    Expression* rhs = args.at(1);

    output << op->name();

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

void prime_terms(auto expr, const string& input, auto op1_ref, auto op2_ref, 
    auto op3_ref, auto val) {
    
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
    auto [state, _0, _1] = CompilationState::create_test_state();

    Expression* expr = Expression::termed(*state.ast_store_, {}, {0,0});

    REQUIRE(expr->terms().size() == 0);

    SUBCASE("String value") {
        Expression* value = Expression::string_literal(*state.ast_store_, "TEST_STRING:oasrpkorsapok", {0,0});
        expr->terms().push_back(value);
        TermedExpressionParser{&state, expr}.run();

        CHECK(expr->expression_type == ExpressionType::string_literal);
        CHECK(expr->string_value() == value->string_value());
    }

    SUBCASE("Number value") {
        Expression* value = Expression::numeric_literal(*state.ast_store_, "234.52", {0,0});
        expr->terms().push_back(value);
        TermedExpressionParser{&state, expr}.run();

        CHECK(expr->expression_type == ExpressionType::numeric_literal);
        CHECK(expr->string_value() == value->string_value());
    }
}

TEST_CASE("TermedExpressionParser should handle binop expressions") {
    auto [state, _0, _1] = CompilationState::create_test_state();
    auto ast = state.ast_store_.get();

    Expression* expr = Expression::termed(*ast, {}, {0,0});

    Expression* val1 = Expression::numeric_literal(*ast, "23", {0,0});
    Expression* val2 = Expression::numeric_literal(*ast, "12", {0,0});

    // create the operator
    auto [op1_ref, op1] = create_operator_helper(state, "+");

    expr->terms() = vector<Expression*>{val1, op1_ref, val2};

    REQUIRE(expr->terms().size() == 3);

    SUBCASE("Simple expression") {
        TermedExpressionParser{&state, expr}.run();

        CHECK(expr->expression_type == ExpressionType::call);
        // NOTE: the operator_ref should be unwrapped

        auto [op, args] = expr->call_value();
        CHECK(op == op1);
        CHECK(args.at(0) == val1);
        CHECK(args.at(1) == val2);
    }

    SUBCASE("should handle higher precedence first") {
        // create operator with lower precedence
        auto [op2_ref, op2] = create_operator_helper(state, "*", 1);
        expr->terms().push_back(op2_ref);

        Expression* val3 = Expression::numeric_literal(*ast, "235", {0,0});
        expr->terms().push_back(val3);

        TermedExpressionParser{&state, expr}.run();

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
        auto [op2_ref, op2] = create_operator_helper(state, "*", 999);
        expr->terms().push_back(op2_ref);

        Expression* val3 = Expression::numeric_literal(*ast, "235", {0,0});

        expr->terms().push_back(val3);

        TermedExpressionParser{&state, expr}.run();

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
    auto [state, _0, _1] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;

    Expression* expr = Expression::termed(ast, {}, {0,0});

    auto [op1_ref, op1] = create_operator_helper(state, "1", 1);
    auto [op2_ref, op2] = create_operator_helper(state, "2", 2);
    auto [op3_ref, op3] = create_operator_helper(state, "3", 3);
    Expression* val = Expression::numeric_literal(ast, "v", {0,0});

    REQUIRE(expr->terms().size() == 0);

    SUBCASE("1 2 1") {
        string input = "1 2 1";
        string expected_pre_order = "11v2vvv";
        
        prime_terms(expr, input, op1_ref, op2_ref, op3_ref, val);

        TermedExpressionParser{&state, expr}.run();
        
        stringstream output;
        traverse_pre_order(expr, output);
        CHECK(output.str() == expected_pre_order);
    }

    SUBCASE("1 2 2 2 1 2 2 2 3 3 2 2 1") {
        string input = "1 2 2 2 1 2 2 2 3 3 2 2 1";
        string expected_pre_order = "111v222vvvv22222vvv33vvvvvv";
        
        prime_terms(expr, input, op1_ref, op2_ref, op3_ref, val);

        TermedExpressionParser{&state, expr}.run();
        
        stringstream output;
        traverse_pre_order(expr, output);
        CHECK(output.str() == expected_pre_order);
    }
}

TEST_CASE("TermedExpressionParser should handle haskell-style call expressions") {
    auto [state, _0, types] = CompilationState::create_test_state();
    RT_Scope globals{};
    AST_Store& ast = *state.ast_store_;

    Expression* expr = Expression::termed(ast, {}, {0,0});
    
    SUBCASE("1 arg") {    
        const Type* function_type = types->get_function_type(&Void, {&String}, true);

        RT_Definition function{"test_f", External{}, function_type, TSL};
        globals.create_identifier(&function);

        Expression* id = Expression::reference(ast, &function, {0,0});
        Expression* arg1 = Expression::string_literal(ast, "", {0,0});

        expr->terms().push_back(id);
        expr->terms().push_back(arg1);

        TermedExpressionParser{&state, expr}.run();
        
        CHECK(expr->expression_type == ExpressionType::call);
        auto [callee, args] = expr->call_value();
        CHECK(callee->name() == "test_f");
        CHECK(args.size() == 1);
        CHECK(args.at(0) == arg1);
    }

    SUBCASE("4 args") {    
        const Type* function_type = types->get_function_type(&Void, 
            {&String, &String, &String, &String}, true);
        
        RT_Definition function{"test_f", External{}, function_type, TSL};
        Expression* id{Expression::reference(ast, &function, {0,0})};
        id->type = function_type;
    
        Expression* arg1 = Expression::string_literal(ast, "", {0,0});
        Expression* arg2 = Expression::string_literal(ast, "", {0,0});
        Expression* arg3 = Expression::string_literal(ast, "", {0,0});
        Expression* arg4 = Expression::string_literal(ast, "", {0,0});

        expr->terms().push_back(id);
        expr->terms().push_back(arg1);
        expr->terms().push_back(arg2);
        expr->terms().push_back(arg3);
        expr->terms().push_back(arg4);

        TermedExpressionParser{&state, expr}.run();
        
        CHECK(expr->expression_type == ExpressionType::call);
        auto [callee, args] = expr->call_value();
        CHECK(callee->name() == "test_f");
        CHECK(args.size() == 4);
        CHECK(args.at(0) == arg1);
        CHECK(args.at(1) == arg2);
        CHECK(args.at(2) == arg3);
        CHECK(args.at(3) == arg4);
    }

    SUBCASE("If the call is not partial, the call expression's type should be the return type") {

        const Type* function_type = types->get_function_type(&Number, {&String}, true);
        
        RT_Definition function{"test_f", External{}, function_type, TSL};
        Expression* ref = Expression::reference(ast, &function, {0,0});

        Expression* arg1 = Expression::string_literal(ast, "", {0,0});

        expr->terms().push_back(ref);
        expr->terms().push_back(arg1);

        TermedExpressionParser{&state, expr}.run();
        
        CHECK(expr->type == &Number);
    }
}

TEST_CASE("Should handle partial application of binary operators") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;

    auto [op_ref, op] = create_operator_helper(state, "-");
    auto val = Expression::numeric_literal(ast, "34", {0,0});

    SUBCASE("left") {
        Expression* expr = Expression::termed(ast, {op_ref, val}, {0,0});
        
        TermedExpressionParser{&state, expr}.run();
        
        CHECK(expr->expression_type == ExpressionType::partial_binop_call_left);
        auto [callee, args] = expr->call_value();
        CHECK(callee == op);
        CHECK(args.size() == 2);
        CHECK(args.at(0)->expression_type == ExpressionType::missing_arg);
        CHECK(args.at(1) == val);
    }

    SUBCASE("right") {
        Expression* expr = Expression::termed(ast, {val, op_ref}, {0,0});
        
        TermedExpressionParser{&state, expr}.run();
        
        CHECK(expr->expression_type == ExpressionType::partial_binop_call_right);
        auto [callee, args] = expr->call_value();
        CHECK(callee == op);
        CHECK(args.size() == 2);
        CHECK(args.at(0) == val);
        CHECK(args.at(1)->expression_type == ExpressionType::missing_arg);
    }
}

TEST_CASE("Should set the type on a non-partial call expression to the return type") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;

    const FunctionType* IntString = types->get_function_type(&String, {&Int}, false);

    auto test_f_expr = Expression{ExpressionType::value, "qwe", IntString, TSL};
    auto test_f = RT_Definition("test_f", &test_f_expr, TSL);

    auto arg = Expression{ExpressionType::numeric_literal, "3", &NumberLiteral, TSL};
    auto reference = Expression::reference(ast, &test_f, TSL);

    auto expr = Expression::termed(ast, {reference, &arg}, TSL);

    TermedExpressionParser{&state, expr}.run();

    CHECK(expr->expression_type == ExpressionType::call);
    CHECK(*expr->type == String);
    CHECK(expr->call_value() == CallExpressionValue{&test_f, {&arg}});
}


TEST_CASE("Should set the type on a non-partial \"operator expression\" to the return type") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;

    const FunctionType* IntString = types->get_function_type(&String, {&Int, &Int}, false);

    auto test_op_expr = Expression{ExpressionType::value, "jii", IntString, TSL};
    auto test_op = RT_Operator::create_binary(">=?", &test_op_expr, 5, 
        Operator::Associativity::left, TSL);

    auto lhs = Expression{ExpressionType::numeric_literal, "3", &NumberLiteral, TSL};
    auto rhs = Expression{ExpressionType::numeric_literal, "7", &NumberLiteral, TSL};

    auto reference = Expression::operator_reference(ast, &test_op, TSL);

    auto expr = Expression::termed(ast, {&lhs, reference, &rhs}, TSL);

    TermedExpressionParser{&state, expr}.run();

    CHECK(expr->expression_type == ExpressionType::call);
    CHECK(expr->call_value() == CallExpressionValue{&test_op, {&lhs, &rhs}});
    CHECK(*expr->type == String);
}

TEST_CASE("Layer2 should handle type specifiers") {
    auto [state, _0, types] = CompilationState::create_test_state();

    auto type_specifier = Expression{ExpressionType::type_reference, &Int, TSL};
    auto value = Expression{ExpressionType::string_literal, "32", &String, TSL};

    SUBCASE("Int \"32\"") {
        auto expr = Expression{ExpressionType::termed_expression, 
            TermedExpressionValue{{&type_specifier, &value}, db_false}, TSL};

        TermedExpressionParser{&state, &expr}.run();

        CHECK(*expr.type == Int);
        CHECK(expr.expression_type == ExpressionType::value);

        CHECK(*value.type == Int);
        CHECK(holds_alternative<maps_Int>(value.value));
        CHECK(get<maps_Int>(value.value) == 32);
    }

    SUBCASE("Int \"32\" + 987") {
        auto op = RT_Operator{"+", External{}, 
            types->get_function_type(&Int, {&Int, &Int}, true),
            {Operator::Fixity::binary}, TSL};
        
        auto op_ref = Expression{ExpressionType::binary_operator_reference, &op, TSL};
        auto rhs = Expression{ExpressionType::numeric_literal, "987", TSL};
        auto expr = Expression{ExpressionType::termed_expression, 
            TermedExpressionValue{{&type_specifier, &value, &op_ref, &rhs}, db_false}, TSL};

        TermedExpressionParser{&state, &expr}.run();

        CHECK(*expr.type == Int);
        CHECK(expr.expression_type == ExpressionType::call);

        auto [callee, args] = expr.call_value();

        CHECK(args.size() == 2);
        CHECK(callee == &op);
        CHECK(*value.type == Int);
        CHECK(holds_alternative<maps_Int>(value.value));
        CHECK(get<maps_Int>(value.value) == 32);
        CHECK(*args.at(0) == value);
    }
}

TEST_CASE("Unary minus by itself should result in a partially applied minus") {
    auto [state, _0, types] = CompilationState::create_test_state();
    auto& ast_store = *state.ast_store_;

    auto value = Expression::numeric_literal(ast_store, "456", TSL);
    auto minus = Expression::minus_sign(ast_store, TSL);

    auto expr = Expression{ExpressionType::termed_expression, 
            TermedExpressionValue{{minus, value}, db_false}, TSL};    

    TermedExpressionParser{&state, &expr}.run();

    CHECK(expr.expression_type == ExpressionType::partially_applied_minus);
    CHECK(std::get<Expression*>(expr.value) == value);
    CHECK(*expr.type == Int);
}