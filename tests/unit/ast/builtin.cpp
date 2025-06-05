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
