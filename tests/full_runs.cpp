#include "doctest.h"

#include <sstream>

#include "mapsc/process_source.hh"

using namespace Maps;
using namespace std;


TEST_CASE("Integration test: parse_source should parse a numberliteral with the correct type") {
    std::stringstream source{"let x = Int 34"};
    
    auto [success, ast, _1] = process_source(source);

    CHECK(success);

    CHECK(ast->globals_->identifier_exists("x"));

    auto x = (*ast->globals_->get_identifier("x"));

    CHECK(*x->get_type() == Int);
}
