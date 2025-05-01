#include "doctest.h"

#include "mapsc/pragma.hh"

using namespace Maps;

TEST_CASE("pragma should be created empty") {
    PragmaStore pragmas{};

    CHECK(pragmas.empty());
    CHECK(pragmas.size() == 0);
}

// !!! known issue
// TEST_CASE("pragmas should work with multiple files") {

// }