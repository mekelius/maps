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
