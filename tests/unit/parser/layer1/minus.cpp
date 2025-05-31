#include <sstream>
#include "doctest.h"

#include "mapsc/parser/layer1.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/builtins.hh"

using namespace Maps;

TEST_CASE("Should recognize minus as a special case") {
    auto types = TypeStore{};
    Scope scope{};
    auto state = CompilationState{get_builtins(), &types};

    std::stringstream source{"-5"};

    auto [success, definition, _1, _2, _3, _4] = run_layer1_eval(state, scope, source);
    CHECK(success);
    CHECK(definition);

    CHECK(std::holds_alternative<const Expression*>((*definition)->const_body()));
    auto expression = std::get<const Expression*>((*definition)->const_body());

    CHECK(expression->expression_type == ExpressionType::layer2_expression);
    auto terms = expression->terms();
    CHECK(terms.size() == 2);
    CHECK(terms.at(0)->expression_type == ExpressionType::minus_sign);
    CHECK(terms.at(1)->expression_type == ExpressionType::known_value);
    CHECK(terms.at(1)->string_value() == "5");
}
