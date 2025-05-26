#include "doctest.h"

#include "mapsc/ast/expression.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/builtins.hh"

#include "mapsc/parser/layer2.hh"

using namespace std;
using namespace Maps;

// TEST_CASE("unary - should work") {
    // auto types = TypeStore{};
    // auto state = CompilationState(get_builtins(), &types);

    // auto op_ref = Expression::operator_reference{&op, TSL};
    // auto value = Expression{ExpressionType::value, TSL, true, &Boolean};
    // auto expr = Expression{ExpressionType::termed_expression, TSL, 
    //     TermedExpressionValue{{&op_ref, &value}}};

    // TermedExpressionParser{&state, &expr}.run();

    // CHECK(state.is_valid);

    // CHECK(expr.expression_type == ExpressionType::call);
    
    // auto [callee, args] = expr.call_value();

    // CHECK(args.size() == 1);
    // CHECK(**args.begin() == value);
    // CHECK(callee == &op);
// }