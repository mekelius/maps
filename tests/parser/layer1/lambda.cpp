#include "doctest.h"

#include "mapsc/parser/parser_layer1.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;

TEST_CASE("Should parse a lambda") {
    auto [state, types, _] = CompilationState::create_test_state();
    RT_Scope scope{};

    std::stringstream source{"\\x -> x"};

    auto result = ParserLayer1{&state, &scope}.run_eval(source);

    CHECK(result.success);
    CHECK(result.top_level_definition);

    auto expression = std::get<const Expression*>((*result.top_level_definition)->const_body());
    CHECK(expression->expression_type == ExpressionType::lambda);
    CHECK(expression->type->is_function());
    CHECK(expression->type->arity() == 1);
}