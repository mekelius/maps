#include "doctest.h"

#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/types/type_store.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;
using namespace std;

TEST_CASE("AST should be empty when created") {
    AST_Store ast{};
    CHECK(ast.empty());
    CHECK(ast.size() == 0);
}

TEST_CASE("Operator::create_binary should create an operator") {
    auto [state, _0, _1] = CompilationState::create_test_state();

    Callable* op_callable = state.ast_store_->allocate_operator(
        RT_Operator::create_binary("+", External{}, Hole, 2, Operator::Associativity::left, TSL));

    CHECK(op_callable->is_operator());
    auto op = dynamic_cast<Operator*>(op_callable);
    CHECK(op);
    
    CHECK(op->is_binary());
}
