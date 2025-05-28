#include "doctest.h"

#include <sstream>
#include <array>

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/ast/reference.hh"
#include "mapsc/ast/termed_expression.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/parser/layer2.hh"
#include "mapsc/logging_options.hh"

using namespace Maps;
using namespace std;


namespace {

inline tuple<Expression*, Definition*> create_operator_helper(CompilationState& state, 
    const string& op_string, unsigned int precedence = 500) {

    const Type* type = state.types_->get_function_type(
        &Number, std::array<const Type* const, 2>{&Number, &Number}, true);
    Definition* op_definition = state.ast_store_->allocate_operator(
        RT_Operator::create_binary(op_string, External{}, type, precedence, 
            RT_Operator::Associativity::left, true, TSL));

    Expression* op_ref = create_operator_reference(*state.ast_store_, op_definition, {0,0});

    return {op_ref, op_definition};
}

} // namespace


TEST_CASE("Should handle partial application of binary operators") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast = *state.ast_store_;

    auto [op_ref, op] = create_operator_helper(state, "-");
    auto val = create_numeric_literal(ast, "34", {0,0});

    SUBCASE("left") {
        Expression* expr = create_termed_testing(ast, {op_ref, val}, {0,0});
        
        run_layer2(state, expr);
        
        CHECK(expr->expression_type == ExpressionType::partial_binop_call_left);
        auto [callee, args] = expr->call_value();
        CHECK(callee == op);
        CHECK(args.size() == 2);
        CHECK(args.at(0)->expression_type == ExpressionType::missing_arg);
        CHECK(args.at(1) == val);
    }

    SUBCASE("right") {
        Expression* expr = create_termed_testing(ast, {val, op_ref}, {0,0});
        
        run_layer2(state, expr);
        
        CHECK(expr->expression_type == ExpressionType::partial_binop_call_right);
        auto [callee, args] = expr->call_value();
        CHECK(callee == op);
        CHECK(args.size() == 2);
        CHECK(args.at(0) == val);
        CHECK(args.at(1)->expression_type == ExpressionType::missing_arg);
    }
}
