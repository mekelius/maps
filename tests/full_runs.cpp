#include "doctest.h"

#include <sstream>

#include "mapsc/process_source.hh"
#include "mapsc/builtins.hh"

using namespace Maps;
using namespace std;

TEST_CASE("Integration test: parse_source should parse a numberliteral with the correct type") {
    TypeStore types{};
    std::stringstream source{"let x = Int 34"};
    
    auto compilation_state = process_source(get_builtins(), &types, source);

    CHECK(compilation_state->is_valid);
    CHECK(compilation_state->globals_->identifier_exists("x"));

    auto x = (*compilation_state->globals_->get_identifier("x"));

    CHECK(*x->get_type() == Int);
}
