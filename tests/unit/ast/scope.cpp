#include "doctest.h"

#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/lambda.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/ast/let_definition.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/pragma.hh"

using namespace Maps;
using namespace std;

namespace {

tuple<CompilationState, shared_ptr<AST_Store>, Scope> setup() {
    auto [state, _] = CompilationState::create_test_state();

    return {
        state,
        state.ast_store_,
        Scope{}
    };
}

} // namespace

struct TestBuiltin {
    std::string name_;

    bool operator<=>(const TestBuiltin&) const = default;
};

constexpr TestBuiltin tb2{"2"};
constexpr TestBuiltin tb1{"1"};

constexpr TBuiltinScope builtins_scope_with_2 {
    &tb2,
    &tb1,
};

constexpr TBuiltinScope builtins_scope_with_1 {
    &tb1
};

static_assert(builtins_scope_with_1.size() == 1);

TEST_CASE("BuiltinScope should order the items") {
    auto it = builtins_scope_with_2.begin();
    CHECK(**it == tb1);
    CHECK(**(++it) == tb2);
}

constexpr DefinitionHeader bex_a{DefinitionType::external_builtin, "a", &Hole, TSL};
constexpr DefinitionHeader bex_b{DefinitionType::external_builtin, "b", &Hole, TSL};

constexpr BuiltinExternalScope builtin_externals_scope {
    &bex_b,
    &bex_a,
};

TEST_CASE("BuiltinScope should order the items") {
    auto it = builtin_externals_scope.begin();
    CHECK(**it == bex_a);
    CHECK(**(++it) == bex_b);
}

TEST_CASE("Should create definitions with proper names") {
    auto [state, types, scope] = setup();
    
    auto expression = create_known_value(state, KnownValue{"123"}, TSL);
    auto [header, body] = create_let_definition(*state.ast_store_, "test", expression, TSL);

    scope.create_identifier(header);

    CHECK(scope.size() == 1);

    CHECK(scope.identifier_exists("test"));
    auto stored_def = scope.get_identifier("test");

    CHECK(stored_def);
    CHECK(**stored_def == *header);
}

TEST_CASE("Should create definitions with stored names") {
    auto [state, types, scope] = setup();
    
    auto expression = create_known_value(state, KnownValue{"123"}, TSL);
    auto [header, body] = create_let_definition(*state.ast_store_, "hmm", expression, TSL);

    scope.create_identifier(header);

    CHECK(scope.identifier_exists("hmm"));
    auto stored_def = scope.get_identifier("hmm");

    CHECK(stored_def);
    CHECK((*stored_def)->name_ == "hmm");
}
