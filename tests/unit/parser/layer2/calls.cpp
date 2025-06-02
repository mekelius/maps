#include "doctest.h"

#include <sstream>
#include <memory>

#include "mapsc/logging_options.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/types/type_defs.hh"

#include "mapsc/ast/function_definition.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/ast/layer2_expression.hh"
#include "mapsc/ast/lambda.hh"
#include "mapsc/ast/reference.hh"

#include "mapsc/parser/layer2.hh"

using namespace Maps;
using namespace std;

namespace {

tuple<CompilationState, shared_ptr<AST_Store>, Scope, unique_ptr<TypeStore>> setup() {
    auto [state, _0, types] = CompilationState::create_test_state();

    return {
        std::move(state),
        state.ast_store_,
        Scope{},
        std::move(types)
    };
}

} // namespace

TEST_CASE("Should be able to call a lambda function") {
    auto [state, ast_store, scope, types] = setup();

    auto [lambda_expr, lambda_def] = create_const_lambda(state, "qwe", 
        std::array<const Type*, 1>{&Int}, TSL);
    auto arg = create_numeric_literal(*ast_store, "3", TSL);
    auto expr = create_layer2_expression_testing(*ast_store, {lambda_expr, arg}, TSL);

    run_layer2(state, expr);

    CHECK(expr->expression_type == ExpressionType::call);
    CHECK(expr->call_value() == CallExpressionValue{lambda_def->header_, {arg}});
}

TEST_CASE("Should be able to call via known value reference to a function") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast_store = *state.ast_store_;
    Scope scope{};

    const FunctionType* IntString = types->get_function_type(&String, {&Int}, false);

    auto test_f_expr = create_known_value(state, "qwe", IntString, TSL);
    CHECK(test_f_expr);    
    auto [test_f, _] = function_definition(state, "test_f", *test_f_expr, TSL);

    auto arg = Expression{ExpressionType::known_value, "3", &NumberLiteral, TSL};
    auto reference = create_reference(ast_store, test_f, TSL);

    auto expr = create_layer2_expression_testing(ast_store, {reference, &arg}, TSL);

    run_layer2(state, expr);

    CHECK(expr->expression_type == ExpressionType::call);
    
    auto [callee, args] = expr->call_value();

    CHECK(*callee == *test_f);
    CHECK(args.size() == 1);
    CHECK(*args.at(0) == arg);
}

TEST_CASE("Should set the type of a call expression to the callee return type") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast_store = *state.ast_store_;
    Scope scope{};

    const FunctionType* IntString = types->get_function_type(&String, {&Int}, false);

    auto test_f_expr = create_known_value(state, "qwe", IntString, TSL);
    CHECK(test_f_expr);
    auto [test_f, _] = function_definition(state, "test_f", *test_f_expr, TSL);

    auto arg = Expression{ExpressionType::known_value, "3", &NumberLiteral, TSL};
    auto reference = create_reference(ast_store, test_f, TSL);

    auto expr = create_layer2_expression_testing(ast_store, {reference, &arg}, TSL);

    run_layer2(state, expr);

    CHECK(expr->expression_type == ExpressionType::call);
    CHECK(*expr->type == String);
}

TEST_CASE("Should set the type on a \"operator expression\" to the return type") {
    auto [state, _0, types] = CompilationState::create_test_state();
    AST_Store& ast_store = *state.ast_store_;

    const FunctionType* IntString = types->get_function_type(&String, std::array{&Int, &Int}, false);

    auto test_op_expr = Expression{ExpressionType::known_value, "jii", IntString, TSL};
    auto [test_op_f, _] = function_definition(state, ">=?_f", &test_op_expr, TSL);
    auto test_op = create_binary_operator(ast_store, ">=?", test_op_f, 5, TSL);

    auto lhs = create_numeric_literal(ast_store, "3", TSL);
    auto rhs = create_numeric_literal(ast_store, "7", TSL);

    auto reference = create_operator_reference(ast_store, test_op, TSL);

    auto expr = create_layer2_expression_testing(ast_store, {lhs, reference, rhs}, TSL);

    run_layer2(state, expr);

    CHECK(expr->expression_type == ExpressionType::call);
    CHECK(expr->call_value() == CallExpressionValue{test_op, {lhs, rhs}});
    CHECK(*expr->type == String);
}
