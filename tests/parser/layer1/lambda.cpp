#include "doctest.h"

#include "mapsc/parser/parser_layer1.hh"
#include "mapsc/compilation_state.hh"

using namespace Maps;

// TEST_CASE("Should parse a lambda") {
//     auto [state, types, _] = CompilationState::create_test_state();

//     std::stringstream source{"\\x -> x"};

//     auto definition = ParserLayer1{&state}.eval_parse(source);

//     CHECK(state.is_valid);
//     CHECK(definition);

//     auto expression = std::get<const Expression*>((*definition)->const_body());
//     CHECK(expression->expression_type == ExpressionType::lambda);
//     CHECK(expression->type->is_function());
//     CHECK(expression->type->arity() == 1);
// }