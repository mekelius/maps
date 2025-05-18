#include "doctest.h"

#include <sstream>

#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/callable.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/dsir/parse_dsir.hh"

using namespace Maps;

TEST_CASE("String literal") {
    auto [state, _1, _2] = CompilationState::create_test_state();
    std::stringstream source{"\"hmm\""};
    
    auto result = eval_parse_dsir(state, source);

    CHECK(state.is_valid);
    CHECK(result);
    CHECK(*(*result)->get_type() == String);

    auto body = std::get<Expression*>((*result)->body_);
    CHECK(body->expression_type == ExpressionType::string_literal);
    CHECK(body->string_value() == "hmm");
}

TEST_CASE("Numeric literal") {
    auto [state, _1, _2] = CompilationState::create_test_state();
    std::stringstream source{"435"};
    
    auto result = eval_parse_dsir(state, source);

    CHECK(state.is_valid);
    CHECK(result);
    CHECK(*(*result)->get_type() == NumberLiteral);

    auto body = std::get<Expression*>((*result)->body_);
    CHECK(body->expression_type == ExpressionType::numeric_literal);
    CHECK(body->string_value() == "435");
}


TEST_CASE("Identifier into reference") {
    auto [state, _1, _2] = CompilationState::create_test_state();

    auto hello = state.ast_store_->allocate_callable({"hello", Undefined{}, TSL});
    state.globals_.create_identifier("hello", hello);

    std::stringstream source{"hello"};
    
    auto result = eval_parse_dsir(state, source);

    CHECK(state.is_valid);
    CHECK(result);
    CHECK(*(*result)->get_type() == Hole);

    auto body = std::get<Expression*>((*result)->body_);
    CHECK(body->expression_type == ExpressionType::reference);
    CHECK(body->reference_value() == hello);
}

// TEST_CASE("let x = \"asd\"") {
//     auto [state, _1, _2] = CompilationState::create_test_state();
//     std::stringstream source{"x"};
    
//     CHECK(parse_dsir(state, source));

//     CHECK(state.globals_.size() == 1);
//     auto x = state.globals_.get_identifier("x");

//     CHECK(x);
//     CHECK((*x)->name_ == "x");
//     CHECK(*(*x)->get_type() == String);

//     auto body = std::get<Expression*>((*x)->body_);
//     CHECK(body->expression_type == ExpressionType::string_literal);
//     CHECK(body->string_value() == "asd");
// }

// TEST_CASE("Int x(Int a, Int b)") {
//     std::stringstream source{"\
//     Int x(Int a, Int b) {\
//         return +(a, b)\
//     }\
//     "};


// }