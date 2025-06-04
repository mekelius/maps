#include "doctest.h"

#include <sstream>
#include <tuple>

#include "mapsc/parser/layer1.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;
using namespace std;

namespace {

tuple<CompilationState, Scope, stringstream> setup(const string& source_str) { 
    auto [state, _1] = CompilationState::create_test_state();

    return {
        std::move(state), 
        Scope{}, 
        stringstream{source_str}
    };
}

} // namespace

TEST_CASE("Should create definitions with proper names") {
    auto [state, scope, source] = setup("let test = 123");
    
    auto result = run_layer1_eval(state, scope, source);

    CHECK(result.success);
    CHECK(scope.size() == 1);
    CHECK(scope.identifier_exists("test"));

    auto definition = *scope.get_identifier("test");
    CHECK(definition->name_ == "test");
}

TEST_CASE("Should create definitions with proper values") {
    auto [state, scope, source] = setup("let test = 123");
    
    auto result = run_layer1_eval(state, scope, source);

    CHECK(result.success);
    CHECK(scope.size() == 1);
    CHECK(scope.identifier_exists("test"));
    CHECK((*scope.get_identifier("test"))->body_);

    auto value = *(*scope.get_identifier("test"))->get_body_value();
    CHECK(holds_alternative<Expression*>(value));

    auto expr = get<Expression*>(value);
    CHECK(expr->expression_type == ExpressionType::known_value);
    CHECK(expr->known_value_value() == KnownValue{"123"});
}