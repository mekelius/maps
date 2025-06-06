#include "doctest.h"

#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/lambda.hh"
#include "mapsc/ast/test_helpers/test_definition.hh"
#include "mapsc/ast/value.hh"
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

TEST_CASE("AST should be empty when created") {
    AST_Store ast{};
    CHECK(ast.empty());
    CHECK(ast.size() == 0);
}

TEST_CASE("Operator::create_binary should create an operator") {
    auto [state, _0] = CompilationState::create_test_state();
    auto& ast_store = *state.ast_store_;

    DefinitionHeader* op_definition = create_testing_binary_operator(ast_store,
        "+", &IntInt_to_Int, 2, Operator::Associativity::left, TSL);

    CHECK(op_definition->is_operator());
    auto op = dynamic_cast<Operator*>(op_definition);
    CHECK(op);
    
    CHECK(op->is_binary());
}

TEST_CASE("Expression_const_lambda should produce a reference") {
    auto [state, ast_store, scope, types] = setup();

    auto [lambda_expr, lambda_def] = create_const_lambda(state, "qwe", 
        std::array<const Type*, 1>{&Int}, TSL);

    CHECK(lambda_expr->expression_type == ExpressionType::reference);
}


TEST_CASE("Should allocate definitions correctly") {
    auto [state, ast_store, _3, _4] = setup();

    auto value = create_known_value(state, 123, TSL);
    auto [header, body] = ast_store->allocate_definition(
        RT_DefinitionHeader{DefinitionType::let_definition, "test", TSL}, value);

    CHECK(std::holds_alternative<Expression*>(body->get_value()));
    CHECK(*std::get<Expression*>(body->get_value()) == *value);
}