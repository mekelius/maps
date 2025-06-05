#include "doctest.h"

#include <array>
#include <tuple>

#include "mapsc/ast/scope.hh"
#include "mapsc/ast/builtin.hh"
#include "mapsc/ast/identifier.hh"
#include "mapsc/ast/reference.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/procedures/name_resolution.hh"

using namespace Maps;
using namespace std;

namespace {

std::tuple<CompilationState, AST_Store&, Scope, std::unique_ptr<TypeStore>> setup() {
    auto [state, types] = CompilationState::create_test_state();
    
    return {
        state,
        *state.ast_store_,
        Scope{},
        std::move(types)
    };
}

} // namespace

constexpr DefinitionHeader bex1{DefinitionType::external_builtin, "jii1", &Unknown, TSL};
constexpr DefinitionHeader bex2{DefinitionType::external_builtin, "jii2", &Unknown, TSL};
constexpr DefinitionHeader bex3{DefinitionType::external_builtin, "jii3", &Unknown, TSL};

constexpr BuiltinExternalScope bexs {
    &bex1,
    &bex2,
    &bex3,
};

constexpr BuiltinValue bev1{"hoo1", "val1", &Unknown};
constexpr BuiltinValue bev2{"hoo2", "val2", &Unknown};
constexpr BuiltinValue bev3{"hoo3", "val3", &Unknown};

constexpr BuiltinValueScope bevs {
    &bev1,
    &bev2,
    &bev3,
};

constexpr auto test_builtins = std::tie(bexs, bevs);

TEST_CASE("Should resolve builtin externals into references") {
    auto [state, ast_store, scope, _] = setup();

    auto identifier = create_identifier(ast_store, &scope, "jii2", TSL);
    CHECK(identifier->expression_type == ExpressionType::identifier);

    CHECK(resolve_identifier(scope, *identifier, test_builtins));
    CHECK(identifier->expression_type == ExpressionType::reference);

    CHECK(*identifier->reference_value() == bex2);
}

TEST_CASE("Should resolve builtin value names into known values") {
    auto [state, ast_store, scope, _] = setup();

    auto identifier = create_identifier(ast_store, &scope, "hoo3", TSL);
    CHECK(identifier->expression_type == ExpressionType::identifier);

    CHECK(resolve_identifier(scope, *identifier, test_builtins));
    CHECK(identifier->expression_type == ExpressionType::known_value);

    CHECK(*identifier->known_value_value() == KnownValue{"val3"});
}