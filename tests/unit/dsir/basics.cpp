#include "doctest.h"

#include <sstream>

#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/dsir/parse_dsir.hh"

using namespace Maps;
using namespace Maps::DSIR;

TEST_CASE("String literal") {
    auto [state, _1, _2] = CompilationState::create_test_state();
    std::stringstream source{"\"hmm\""};
    
    auto [success, top_level, definitions, _] = eval_parse_dsir(state, source);

    CHECK(success);
    CHECK(top_level);
    CHECK(*(*top_level)->get_type() == String);

    auto body = std::get<const Expression*>((*top_level)->const_body());
    CHECK(body->expression_type == ExpressionType::known_value);
    CHECK(body->string_value() == "hmm");
}

TEST_CASE("Numeric literal") {
    auto [state, _1, _2] = CompilationState::create_test_state();
    std::stringstream source{"435"};
    
    auto [success, top_level, definitions, _] = eval_parse_dsir(state, source);

    CHECK(top_level);
    CHECK(*(*top_level)->get_type() == NumberLiteral);

    auto body = std::get<const Expression*>((*top_level)->const_body());
    CHECK(body->expression_type == ExpressionType::known_value);
    CHECK(body->string_value() == "435");
}

TEST_CASE("Identifier into reference") {
    auto [state, _1, _2] = CompilationState::create_test_state();

    auto hello = state.ast_store_->allocate_definition({"hello", Undefined{}, TSL});
    state.globals_.create_identifier("hello", hello);

    std::stringstream source{"hello"};
    
    auto [success, top_level, definitions, _] = eval_parse_dsir(state, source);

    CHECK(top_level);
    CHECK(*(*top_level)->get_type() == Unknown);

    auto body = std::get<const Expression*>((*top_level)->const_body());
    CHECK(body->expression_type == ExpressionType::reference);
    CHECK(body->reference_value() == hello);
}

TEST_CASE("Definition") {
    auto [state, _1, _2] = CompilationState::create_test_state();
    std::stringstream source{"let x = 23"};
    
    auto [success, top_level, definitions, _] = parse_dsir(state, source);

    CHECK(success);
    CHECK(!top_level);
    CHECK(definitions.size() == 1);
    auto x = definitions.get_identifier("x");

    CHECK(x);
    CHECK((*x)->name() == "x");
    CHECK(*(*x)->get_type() == NumberLiteral);

    auto body = std::get<const Expression*>((*x)->const_body());
    CHECK(body->expression_type == ExpressionType::known_value);
    CHECK(body->string_value() == "23");
}

// TEST_CASE("Reference into a previous definition") {
//     auto [state, _1, _2] = CompilationState::create_test_state();

//     SUBCASE("internal previous definition") {
//         std::stringstream source{"let x = 2"};
//     }
// }



TEST_CASE("Shouldn't insert into globals") {
    auto [state, _1, _2] = CompilationState::create_test_state();

    SUBCASE("Top level eval shouldn't") {
        std::stringstream source{"\"hmm\""};
        
        auto [success, top_level, definitions, _] = eval_parse_dsir(state, source);
        
        CHECK(success);
        CHECK(top_level);
        CHECK(definitions.empty());
    }

    SUBCASE("Definition in eval shouldn't") {
        std::stringstream source{"let x = 2"};
        
        auto [success, top_level, definitions, _] = eval_parse_dsir(state, source);
        
        CHECK(success);
        CHECK(!top_level);
        CHECK(definitions.size() == 1);
    }

    SUBCASE("Definition not in eval shouldn't") {
        std::stringstream source{"let x = 2"};
        
        auto [success, top_level, definitions, _] = parse_dsir(state, source);
        
        CHECK(success);
        CHECK(!top_level);
        CHECK(definitions.size() == 1);
    }
}



// TEST_CASE("Int x(Int a, Int b)") {
//     std::stringstream source{"\
//     Int x(Int a, Int b) {\
//         return +(a, b)\
//     }\
//     "};


// }