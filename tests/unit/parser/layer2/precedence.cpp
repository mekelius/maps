#include "doctest.h"

#include <sstream>

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/value.hh"

#include "mapsc/ast/layer2_expression.hh"
#include "mapsc/ast/reference.hh"
#include "mapsc/ast/test_helpers/test_definition.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/parser/layer2.hh"
#include "mapsc/logging_options.hh"

using namespace Maps;
using namespace std;

// --------------- HELPERS ---------------

namespace {

inline tuple<Expression*, DefinitionHeader*> create_operator_helper(CompilationState& state, 
    const string& op_string, unsigned int precedence = 500) {

    const Type* type = state.types_->get_function_type(&NumberLiteral, {&NumberLiteral, &NumberLiteral}, true);
    auto op_definition = create_testing_binary_operator(*state.ast_store_, op_string, 
        type, precedence, Operator::Associativity::left, TSL);

    Expression* op_ref = create_operator_reference(*state.ast_store_, op_definition, TSL);

    return {op_ref, op_definition};
}

// takes a parsed binop expression tree and traverses it in preorder
void traverse_pre_order(Expression* tree, ostream& output) {
    auto [op, args] = tree->call_value();

    Expression* lhs = args.at(0);
    Expression* rhs = args.at(1);

    output << op->name_string();

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

} // namespace

TEST_CASE("TermedExpressionParser should handle binop expressions") {
    auto [state, _0, _1] = CompilationState::create_test_state();
    auto ast = state.ast_store_.get();

    Expression* expr = create_layer2_expression_testing(*ast, {}, {0,0});

    Expression* val1 = create_numeric_literal(*ast, "23", TSL);
    Expression* val2 = create_numeric_literal(*ast, "12", TSL);

    // create the operator
    auto [op1_ref, op1] = create_operator_helper(state, "+");

    expr->terms() = vector<Expression*>{val1, op1_ref, val2};

    REQUIRE(expr->terms().size() == 3);

    SUBCASE("Simple expression") {
        run_layer2(state, expr);

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

        Expression* val3 = create_numeric_literal(*ast, "235", {0,0});
        expr->terms().push_back(val3);

        run_layer2(state, expr);

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

        Expression* val3 = create_numeric_literal(*ast, "235", {0,0});

        expr->terms().push_back(val3);

        run_layer2(state, expr);

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

    Expression* expr = create_layer2_expression_testing(ast, {}, {0,0});

    auto [op1_ref, op1] = create_operator_helper(state, "1", 1);
    auto [op2_ref, op2] = create_operator_helper(state, "2", 2);
    auto [op3_ref, op3] = create_operator_helper(state, "3", 3);
    Expression* val = create_numeric_literal(ast, "v", {0,0});

    REQUIRE(expr->terms().size() == 0);

    SUBCASE("1 2 1") {
        string input = "1 2 1";
        string expected_pre_order = "11v2vvv";
        
        prime_terms(expr, input, op1_ref, op2_ref, op3_ref, val);

        run_layer2(state, expr);
        
        stringstream output;
        traverse_pre_order(expr, output);
        CHECK(output.str() == expected_pre_order);
    }

    SUBCASE("1 2 2 2 1 2 2 2 3 3 2 2 1") {
        string input = "1 2 2 2 1 2 2 2 3 3 2 2 1";
        string expected_pre_order = "111v222vvvv22222vvv33vvvvvv";
        
        prime_terms(expr, input, op1_ref, op2_ref, op3_ref, val);

        run_layer2(state, expr);
        
        stringstream output;
        traverse_pre_order(expr, output);
        CHECK(output.str() == expected_pre_order);
    }
}
