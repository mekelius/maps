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

tuple<CompilationState, shared_ptr<AST_Store>> setup() {
    auto [state, _0] = CompilationState::create_test_state();

    return {
        state,
        state.ast_store_,
    };
}

} // namespace

TEST_CASE("Definitions should know if they are a known value") {
    // auto [state, ast_store, _3] = setup();
    auto [state, ast_store] = setup();//CompilationState::create_test_state();
    // auto& ast_store = *state.ast_store_;

    auto value = create_known_value(state, 123, TSL);
    auto [definition, _] = ast_store->allocate_definition(
        DefinitionHeader{DefinitionType::let_definition, "test", TSL}, value);

    CHECK(definition->is_known_scalar_value());
}


TEST_CASE("Should be able to create a let definition") {
    auto [state, types] = CompilationState::create_test_state();
    REQUIRE(types->empty());

    auto IntIntInt = types->get_function_type(&Int, {&Int, &Int}, true);
    auto [dummy_definition, _2] = create_let_definition(*state.ast_store_, IntIntInt, TSL);

    CHECK(*dummy_definition->get_type() == *IntIntInt);
}

TEST_CASE("Should create definitions with proper names") {
    auto [state, types] = setup();
    
    auto expression = create_known_value(state, KnownValue{"123"}, TSL);
    auto [header, body] = create_let_definition(*state.ast_store_, "test", expression, TSL);

    CHECK(header->name_ == "test");
}

TEST_CASE("Should create definitions with stored names") {
    auto [state, types] = setup();
    
    std::string name = "test";

    auto expression = create_known_value(state, KnownValue{"123"}, TSL);
    auto [header, body] = create_let_definition(*state.ast_store_, name, expression, TSL);

    name = "kk";
    CHECK(header->name_ == "test");
}

TEST_CASE("Should create definitions with proper values") {
    auto [state, types] = setup();
    
    auto expression = create_known_value(state, KnownValue{"123"}, TSL);
    auto [header, body] = create_let_definition(*state.ast_store_, "test", expression, TSL);

    CHECK(header->name_ == "test");

    auto value = body->get_value();
    CHECK(holds_alternative<Expression*>(value));

    auto expr = get<Expression*>(value);
    CHECK(expr->expression_type == ExpressionType::known_value);
    CHECK(expr->known_value_value() == KnownValue{"123"});
}