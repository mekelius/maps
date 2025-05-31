#include "doctest.h"

#include "mapsc/ast/expression.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/ast/layer2_expression.hh"
#include "mapsc/source.hh"
#include "mapsc/parser/layer2.hh"

using namespace std;
using namespace Maps;

// TEST_CASE("prefix purely unary operator") {
//     auto [state, _0, types] = CompilationState::create_test_state();

//     auto op = Operator{"!", types->get_function_type(&Boolean, array{&Boolean}, true), 
//         {Operator::Fixity::unary_prefix}, true, TSL};
//     auto op_ref = Expression{ExpressionType::prefix_operator_reference, &op, TSL};
//     auto value = Expression{ExpressionType::known_value, true, &Boolean, TSL};
//     auto expr = create_layer2_expression_testing(*state.ast_store_, {&op_ref, &value}, TSL);

//     run_layer2(state, expr);

//     CHECK(expr->expression_type == ExpressionType::call);
    
//     auto [callee, args] = expr->call_value();

//     CHECK(args.size() == 1);
//     CHECK(**args.begin() == value);
//     CHECK(callee == &op);
// }

// TEST_CASE("postfix purely unary operator") {
//     auto [state, _0, types] = CompilationState::create_test_state();

//     auto op = Operator{"!", types->get_function_type(&Boolean, array{&Boolean}, true), 
//         {Operator::Fixity::unary_postfix}, true, TSL};
//     auto op_ref = Expression{ExpressionType::postfix_operator_reference, &op, op.get_type(), TSL};
//     auto value = Expression{ExpressionType::known_value, true, &Boolean, TSL};
//     auto expr = create_layer2_expression_testing(*state.ast_store_, {&value, &op_ref}, TSL);

//     run_layer2(state, expr);

//     CHECK(expr->expression_type == ExpressionType::call);
    
//     auto [callee, args] = expr->call_value();

//     CHECK(args.size() == 1);
//     CHECK(**args.begin() == value);
//     CHECK(callee == &op);
// }

// TEST_CASE("Chained unary prefixes") {
//     auto [state, _0, types] = CompilationState::create_test_state();

//     auto op1 = Operator{"!", types->get_function_type(&Boolean, 
//         std::array<const Type* const, 1>{&Boolean}, true), {Operator::Fixity::unary_prefix}, true, TSL};
//     auto op2 = Operator{"-", types->get_function_type(&Boolean, 
//         std::array<const Type* const, 1>{&Boolean}, true), {Operator::Fixity::unary_prefix}, true, TSL};

//     auto op_ref1 = Expression{ExpressionType::prefix_operator_reference, &op1, op1.get_type(), TSL};
//     auto op_ref2 = Expression{ExpressionType::prefix_operator_reference, &op2, op2.get_type(), TSL};
//     auto op_ref3 = Expression{ExpressionType::prefix_operator_reference, &op1, op1.get_type(), TSL};

//     auto value = Expression{ExpressionType::known_value, true, &Boolean, TSL};
//     auto expr = create_layer2_expression_testing(*state.ast_store_, 
//         {&op_ref3, &op_ref2, &op_ref1, &value}, TSL);

//     run_layer2(state, expr);

//     CHECK(expr->expression_type == ExpressionType::call);
    
//     auto [callee1, args1] = expr->call_value();
//     CHECK(*callee1 == op1);
//     CHECK(args1.size() == 1);
//     auto arg1 = *args1.begin();
//     CHECK(arg1->expression_type == ExpressionType::call);

//     auto [callee2, args2] = arg1->call_value();
//     CHECK(*callee2 == op2);
//     CHECK(args2.size() == 1);
//     auto arg2 = *args2.begin();
//     CHECK(arg2->expression_type == ExpressionType::call);

//     auto [callee3, args3] = arg2->call_value();
//     CHECK(*callee3 == op1);
//     CHECK(args3.size() == 1);
//     auto arg3 = *args3.begin();
//     CHECK(arg3->expression_type == ExpressionType::known_value);
//     CHECK(*arg3 == value);
// }