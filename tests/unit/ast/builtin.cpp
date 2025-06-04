#include "doctest.h"

#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/value.hh"
#include "mapsc/ast/builtin.hh"
#include "mapsc/ast/test_helpers/test_definition.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;
using namespace std;

namespace {

tuple<CompilationState, shared_ptr<AST_Store>, Scope, unique_ptr<TypeStore>> setup() {
    auto [state, types] = CompilationState::create_test_state();

    return {
        std::move(state),
        state.ast_store_,
        Scope{},
        std::move(types)
    };
}

} // namespace

constexpr Builtin test_value = create_builtin_known_value("name", "qwe");

TEST_CASE("Should be able to create builtins") {
    CHECK(test_value.header.name_ == "name");
    CHECK(std::get<const char*>(test_value.value) == "qwe");
}