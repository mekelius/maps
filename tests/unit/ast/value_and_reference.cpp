#include "doctest.h"

#include "mapsc/ast/value.hh"
#include "mapsc/ast/definition_body.hh"
#include "mapsc/ast/let_definition.hh"
#include "mapsc/ast/test_helpers/test_definition.hh"
#include "mapsc/ast/reference.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;
using namespace std;

namespace {

std::tuple<CompilationState, AST_Store&> setup() {
    auto [state, _1, _2] = CompilationState::create_test_state();
    
    return {
        state,
        *state.ast_store_
    };
}

} // namespace

TEST_CASE("create_reference should create a known value reference if called with a known value") {
    auto [state, ast_store] = setup();
    
    auto value = create_known_value(state, 24, TSL);
    auto def = create_let_definition(ast_store, value, TSL);

    auto ref = create_reference(ast_store, def, TSL);
    CHECK(ref->expression_type == ExpressionType::known_value_reference);
    CHECK(ref->reference_value() == def->header_);
}

