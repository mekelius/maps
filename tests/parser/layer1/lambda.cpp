#include "doctest.h"

#include "mapsc/parser/parser_layer1.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;

TEST_CASE("Should parse lambdas") {
    auto [state, types, _] = CompilationState::create_test_state();
    RT_Scope scope{};

    ParserLayer1 layer1{&state, &scope};

    REQUIRE(state.ast_store_->empty());
    REQUIRE(scope.empty());

    SUBCASE("\\x -> x") {
        std::stringstream source{"\\x -> x"};

        auto result = layer1.run_eval(source);

        CHECK(result.success);
        CHECK(result.top_level_definition);

        auto expression = std::get<const Expression*>((*result.top_level_definition)->const_body());
        CHECK(expression->expression_type == ExpressionType::lambda);
        CHECK(expression->type->is_function());
        CHECK(expression->type->arity() == 1);
    }

    // SUBCASE("\\x y -> x + y") {
    //     std::stringstream source{"\\x y -> x + y"};

    //     auto result = layer1.run_eval(source);

    //     CHECK(result.success);
    //     CHECK(result.top_level_definition);

    //     auto expression = std::get<const Expression*>((*result.top_level_definition)->const_body());
    //     CHECK(expression->expression_type == ExpressionType::lambda);
    //     CHECK(expression->type->is_function());
    //     CHECK(expression->type->arity() == 1);
    // }
}
