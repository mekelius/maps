#include "doctest.h"

#include "mapsc/types/type.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;

TEST_CASE("Should be able to compare types") {
    TypeStore types{};

    SUBCASE("Simply") {
        CHECK(Float == Float);
        CHECK(Int != Float);
    }

    SUBCASE("From store to global") {
        CHECK(Float == **types.get("Float"));
        CHECK(**types.get("Float") == Float);

        CHECK(Int != **types.get("Float"));
        CHECK(**types.get("Int") != Float);
    }
}

TEST_CASE("Should be able to compare types of expressions") {
    auto [state, _1, types] = CompilationState::create_test_state();
    AST_Store& store = *state.ast_store_;

    SUBCASE("Expression to global") {
        auto int_e = Expression::known_value(state, 123, TSL);
        auto string_e = Expression::known_value(state, "qweqe", TSL);

        CHECK(*int_e->type == Int);
        CHECK(*string_e->type == String);
    }
}

